package auth

import (
	"database/sql"
	"encoding/json"
	"net/http"
"log"
"fmt"
	"time"
	"math/rand"
	_ "github.com/mattn/go-sqlite3"
)

type KeyManagementRequest struct {
	Key      string `json:"key"`
	Note     string `json:"note"`
	Duration int    `json:"duration"`
}
func GenerateKeyHandler(db *sql.DB) http.HandlerFunc {
    return func(w http.ResponseWriter, r *http.Request) {
        if r.Header.Get("Content-Type") != "application/json" {
            http.Error(w, "Invalid content type", http.StatusBadRequest)
            return
        }

        var keyRequest struct {
            Key      string `json:"key"`
            Note     string `json:"note"`
            Duration int    `json:"duration"` 
        }
        if err := json.NewDecoder(r.Body).Decode(&keyRequest); err != nil {
            http.Error(w, fmt.Sprintf("Failed to parse JSON: %v", err), http.StatusBadRequest)
            return
        }

        if keyRequest.Key == "" || keyRequest.Duration <= 0 {
            http.Error(w, "Missing or invalid key data", http.StatusBadRequest)
            return
        }

        expirationTime := time.Now().Add(time.Duration(keyRequest.Duration) * time.Hour)

        result, err := db.Exec("INSERT INTO keys (key, status, duration, note, expires_at) VALUES (?, ?, ?, ?, ?)",
            keyRequest.Key, "active", keyRequest.Duration, keyRequest.Note, expirationTime)
        
        if err != nil {
            log.Printf("Error inserting key into the database: %v", err)
            http.Error(w, fmt.Sprintf("Failed to add key: %v", err), http.StatusInternalServerError)
            return
        }

        lastInsertID, err := result.LastInsertId()
        if err != nil {
            log.Printf("Error retrieving last insert ID: %v", err)
            http.Error(w, "Failed to retrieve last insert ID", http.StatusInternalServerError)
            return
        }

        w.WriteHeader(http.StatusOK)
        json.NewEncoder(w).Encode(map[string]interface{}{
            "message": "Key added successfully",
            "key":     keyRequest.Key,
            "id":      lastInsertID, 
        })
    }
}





func AddKeyHandler(db *sql.DB) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("Content-Type") != "application/json" {
			http.Error(w, "Invalid content type", http.StatusBadRequest)
			return
		}

		var keyRequest KeyManagementRequest
		if err := json.NewDecoder(r.Body).Decode(&keyRequest); err != nil {
			http.Error(w, "Failed to parse JSON", http.StatusBadRequest)
			return
		}

		expirationTime := time.Now().Add(time.Duration(keyRequest.Duration) * time.Hour)

		_, err := db.Exec("INSERT INTO keys (key, status, duration, note, expires_at) VALUES (?, ?, ?, ?, ?)",
			keyRequest.Key, "active", keyRequest.Duration, keyRequest.Note, expirationTime)
		if err != nil {
			http.Error(w, "Failed to add key", http.StatusInternalServerError)
			return
		}

		w.WriteHeader(http.StatusOK)
		json.NewEncoder(w).Encode(map[string]string{"message": "Key added successfully"})
	}
}

func FetchKeysHandler(db *sql.DB) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		currentTime := time.Now()

		rows, err := db.Query("SELECT key, status, duration, note, expires_at FROM keys WHERE status = 'active' AND expires_at > ?", currentTime)
		if err != nil {
			http.Error(w, "Failed to fetch keys", http.StatusInternalServerError)
			return
		}
		defer rows.Close()

		var keys []map[string]interface{}
		for rows.Next() {
			var key, status, note string
			var duration int
			var expiresAt time.Time
			if err := rows.Scan(&key, &status, &duration, &note, &expiresAt); err != nil {
				http.Error(w, "Failed to scan keys", http.StatusInternalServerError)
				return
			}
			keys = append(keys, map[string]interface{}{
				"key":      key,
				"status":   status,
				"duration": duration,
				"note":     note,
				"expiresAt": expiresAt,
			})
		}

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(map[string]interface{}{"keys": keys})
	}
}

func RevokeKeyHandler(db *sql.DB) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("Content-Type") != "application/json" {
			http.Error(w, "Invalid content type", http.StatusBadRequest)
			return
		}

		var keyRequest struct {
			Key string `json:"key"`
		}
		if err := json.NewDecoder(r.Body).Decode(&keyRequest); err != nil {
			http.Error(w, "Failed to parse JSON", http.StatusBadRequest)
			return
		}

		_, err := db.Exec("UPDATE keys SET status = 'revoked' WHERE key = ?", keyRequest.Key)
		if err != nil {
			http.Error(w, "Failed to revoke key", http.StatusInternalServerError)
			return
		}

		w.WriteHeader(http.StatusOK)
		json.NewEncoder(w).Encode(map[string]string{"message": "Key revoked successfully"})
	}
}

func randomString(length int) string {
	const charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	var result []byte
	for i := 0; i < length; i++ {
		result = append(result, charset[rand.Intn(len(charset))])
	}
	return string(result)
}
