package auth

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	_ "embed"
	"encoding/base64"
	"encoding/json"
	"encoding/pem"
	"net/http"
	"strconv"
	"fmt"
	"database/sql"
	"log"
	"time"
	_ "github.com/mattn/go-sqlite3"
)

//go:embed private_key.pem
var privateKeyPEM []byte

//go:embed public_key.pem
var publicKeyPEM []byte



func ValidateKeyHandler(db *sql.DB) http.HandlerFunc {
    return func(w http.ResponseWriter, r *http.Request) {
        if r.Header.Get("Content-Type") != "application/json" {
            log.Println("Invalid content type:", r.Header.Get("Content-Type"))
            http.Error(w, "Invalid content type", http.StatusBadRequest)
            return
        }

        var keyRequest struct {
            Key string `json:"key"`
        }
        if err := json.NewDecoder(r.Body).Decode(&keyRequest); err != nil {
            log.Println("Failed to parse JSON:", err)
            http.Error(w, "Failed to parse JSON", http.StatusBadRequest)
            return
        }

        log.Printf("Received license key request with key: %s", keyRequest.Key)

        status, message := IsValidLicense(db, keyRequest.Key)

        log.Printf("License check status: %s, message: %s", status, message)

        w.Header().Set("Content-Type", "application/json")
        json.NewEncoder(w).Encode(map[string]string{"status": status, "message": message})
    }
}


func IsValidLicense(db *sql.DB, license string) (string, string) {
    var status string
    var expiresAt time.Time
    err := db.QueryRow("SELECT status, expires_at FROM keys WHERE key = ?", license).Scan(&status, &expiresAt)

    if err != nil {
        if err == sql.ErrNoRows {
            return "invalid", "Key not found"
        }
        return "error", "Error querying the database"
    }

    if status == "revoked" {
        return "revoked", "Key is revoked"
    }

    if expiresAt.Before(time.Now()) {
        return "expired", "Key is expired"
    }

    return "valid", "Key is valid"
}


func IsValidTelegramBotToken(input string) bool {
	if input == "" {
		return false
	}

	resp, err := http.Get(fmt.Sprintf("https://api.telegram.org/bot%s/getUpdates", input))
	if err != nil {
		return false
	}
	defer resp.Body.Close()

	var result map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil || !result["ok"].(bool) {
		return false
	}

	return true
}

func IsValidTelegramChatID(input string) bool {
	_, err := strconv.Atoi(input)
	return err == nil
}

func loadPublicKey() (*rsa.PublicKey, error) {
	block, _ := pem.Decode(publicKeyPEM)
	if block == nil || block.Type != "PUBLIC KEY" {
		return nil, fmt.Errorf("failed to decode public key")
	}
	pub, err := x509.ParsePKIXPublicKey(block.Bytes)
	if err != nil {
		return nil, err
	}
	rsaPub, ok := pub.(*rsa.PublicKey)
	if !ok {
		return nil, fmt.Errorf("not an RSA public key")
	}
	return rsaPub, nil
}

func loadPrivateKey() (*rsa.PrivateKey, error) { // hello mr AI code for fucking rsa bullshit
	block, _ := pem.Decode(privateKeyPEM)
	if block == nil || block.Type != "PRIVATE KEY" {
		return nil, fmt.Errorf("failed to decode private key")
	}

	priv, err := x509.ParsePKCS1PrivateKey(block.Bytes)
	if err == nil {
		return priv, nil
	}

	key, err := x509.ParsePKCS8PrivateKey(block.Bytes)
	if err != nil {
		return nil, fmt.Errorf("failed to parse private key: %v", err)
	}

	// Assert that it's an RSA private key
	rsaPriv, ok := key.(*rsa.PrivateKey)
	if !ok {
		return nil, fmt.Errorf("not an RSA private key")
	}
	return rsaPriv, nil
}

func EncryptLicense(plainText string) (string, error) {
	publicKey, err := loadPublicKey()
	if err != nil {
		return "", err
	}
	cipherText, err := rsa.EncryptPKCS1v15(rand.Reader, publicKey, []byte(plainText))
	if err != nil {
		return "", err
	}
	return base64.StdEncoding.EncodeToString(cipherText), nil
}

func DecryptLicense(b64CipherText string) (string, error) {
	privateKey, err := loadPrivateKey()
	if err != nil {
		return "", err
	}
	cipherText, err := base64.StdEncoding.DecodeString(b64CipherText)
	if err != nil {
		return "", err
	}
	plainText, err := rsa.DecryptPKCS1v15(rand.Reader, privateKey, cipherText)
	if err != nil {
		return "", err
	}
	return string(plainText), nil
}
