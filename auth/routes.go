package auth

import (
	"net/http"
	"os"
	"log"
)


func ServeHTML(filename string) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		fullPath := "./web/templates/" + filename
		log.Printf("Attempting to serve file: %s", fullPath)
		if _, err := os.Stat(fullPath); os.IsNotExist(err) {
			log.Printf("File not found: %s", fullPath)
			http.Error(w, "404 Page Not Found", http.StatusNotFound)
			return
		}
		log.Printf("Serving file: %s", fullPath)
		http.ServeFile(w, r, fullPath)
	}
}
