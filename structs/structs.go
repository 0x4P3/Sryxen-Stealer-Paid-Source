package structs

type BuildRequest struct {
	License          string `json:"license"`
	TelegramBotToken string `json:"telegramBotToken"`
	TelegramChatID   string `json:"telegramChatID"`
}

type TelegramRequest struct {
	ChatID    string `json:"chat_id"`
	Text      string `json:"text"`
	ParseMode string `json:"parse_mode"`
}
