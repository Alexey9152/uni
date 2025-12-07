import java.io.*;
import java.util.*;

class TextFormatter {
    private int width;

    public TextFormatter(int width) {
        this.width = width;
    }

    public List<String> formatText(String text) {
        List<String> result = new ArrayList<>();
        String[] words = text.split("\\s+");
        List<String> currentLine = new ArrayList<>();
        int currentLength = 0;

        for (String word : words) {
            if (word.isEmpty()) continue;

            // Если слово слишком длинное, разбиваем его
            if (word.length() > width) {
                // Сначала добавляем текущую строку, если она не пустая
                if (!currentLine.isEmpty()) {
                    result.add(buildLine(currentLine, currentLength));
                    currentLine.clear();
                    currentLength = 0;
                }

                // Разбиваем длинное слово на части
                List<String> wordParts = splitLongWord(word, width);
                for (int i = 0; i < wordParts.size(); i++) {
                    String part = wordParts.get(i);
                    // Для всех частей кроме последней добавляем дефис
                    if (i < wordParts.size() - 1) {
                        part += "-";
                    }

                    // Если часть не помещается в текущую строку
                    if (part.length() > width ||
                            (!currentLine.isEmpty() &&
                                    currentLength + part.length() + currentLine.size() > width)) {
                        if (!currentLine.isEmpty()) {
                            result.add(buildLine(currentLine, currentLength));
                            currentLine.clear();
                            currentLength = 0;
                        }
                    }

                    // Если часть всё ещё слишком длинная, разбиваем дальше
                    while (part.length() > width) {
                        String subPart = part.substring(0, width - 1) + "-";
                        result.add(subPart);
                        part = part.substring(width - 1);
                    }

                    currentLine.add(part);
                    currentLength += part.length();
                }
            } else {
                // Обычное слово
                if (currentLength + word.length() + currentLine.size() > width) {
                    if (!currentLine.isEmpty()) {
                        result.add(buildLine(currentLine, currentLength));
                        currentLine.clear();
                        currentLength = 0;
                    }
                }
                currentLine.add(word);
                currentLength += word.length();
            }
        }

        // Добавляем последнюю строку
        if (!currentLine.isEmpty()) {
            String lastLine = String.join(" ", currentLine);
            if (lastLine.length() > width) {
                // Если последняя строка слишком длинная, разбиваем её
                List<String> lastLines = splitLineToWidth(lastLine, width);
                for (int i = 0; i < lastLines.size(); i++) {
                    if (i < lastLines.size() - 1) {
                        result.add(lastLines.get(i));
                    } else {
                        // Последнюю часть выравниваем по ширине
                        result.add(buildLine(Arrays.asList(lastLines.get(i).split("\\s+")),
                                lastLines.get(i).replaceAll("\\s+", "").length()));
                    }
                }
            } else {
                // Выравниваем последнюю строку по ширине
                result.add(buildLine(currentLine, currentLength));
            }
        }

        return result;
    }

    private List<String> splitLongWord(String word, int maxWidth) {
        List<String> parts = new ArrayList<>();
        int start = 0;

        while (start < word.length()) {
            int end = Math.min(start + maxWidth, word.length());

            // Стараемся разбить по слогам (после гласной)
            if (end < word.length()) {
                int breakPoint = findBreakPoint(word, start, end);
                if (breakPoint > start && breakPoint < word.length()) {
                    end = breakPoint;
                }
            }

            parts.add(word.substring(start, end));
            start = end;
        }

        return parts;
    }

    private List<String> splitLineToWidth(String line, int maxWidth) {
        List<String> result = new ArrayList<>();
        String[] words = line.split("\\s+");
        List<String> currentLine = new ArrayList<>();
        int currentLength = 0;

        for (String word : words) {
            if (currentLength + word.length() + currentLine.size() > maxWidth) {
                if (!currentLine.isEmpty()) {
                    result.add(buildLine(currentLine, currentLength));
                    currentLine.clear();
                    currentLength = 0;
                }
            }
            currentLine.add(word);
            currentLength += word.length();
        }

        if (!currentLine.isEmpty()) {
            result.add(String.join(" ", currentLine));
        }

        return result;
    }

    private int findBreakPoint(String word, int start, int end) {
        // Правила переноса для русского языка:
        // 1. Не оставлять одну букву
        // 2. Переносить после гласной
        // 3. Не разрывать буквы й,ь,ъ

        String vowels = "аеёиоуыэюяАЕЁИОУЫЭЮЯ";
        String nonBreakable = "йьъЙЬЪ";

        // Ищем позицию для переноса с конца
        for (int i = end - 1; i > start + 1; i--) {
            char currentChar = word.charAt(i);
            char prevChar = word.charAt(i - 1);

            // Не разрываем после й,ь,ъ
            if (nonBreakable.indexOf(prevChar) >= 0) {
                continue;
            }

            // Переносим после гласной
            if (vowels.indexOf(prevChar) >= 0) {
                return i;
            }
        }

        // Если не нашли хорошего места, разбиваем пополам
        return start + (end - start) / 2;
    }

    private String buildLine(List<String> words, int wordsLength) {
        if (words.size() == 1) {
            // Если слово одно, дополняем пробелами справа
            return String.format("%-" + width + "s", words.get(0));
        }

        int totalSpaces = width - wordsLength;
        int gaps = words.size() - 1;
        int spacesPerGap = totalSpaces / gaps;
        int extraSpaces = totalSpaces % gaps;

        StringBuilder line = new StringBuilder();
        for (int i = 0; i < words.size(); i++) {
            line.append(words.get(i));
            if (i < gaps) {
                // Добавляем базовые пробелы
                for (int j = 0; j < spacesPerGap; j++) {
                    line.append(' ');
                }
                // Добавляем дополнительные пробелы
                if (i < extraSpaces) {
                    line.append(' ');
                }
            }
        }

        // Проверяем, что строка имеет нужную длину
        String result = line.toString();
        if (result.length() != width) {
            // Если нет, дополняем пробелами
            return String.format("%-" + width + "s", result);
        }

        return result;
    }
}

class FileProcessor {
    public String readFile(String filename) throws IOException {
        File file = new File(filename);
        if (!file.exists()) {
            createSampleFile(filename);
        }

        StringBuilder content = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (!content.isEmpty()) {
                    content.append(" ");
                }
                content.append(line.trim().replaceAll("\\s+", " "));
            }
        }
        return content.toString();
    }

    public void writeFile(String filename, List<String> lines) throws IOException {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filename))) {
            for (String line : lines) {
                writer.write(line);
                writer.newLine();
            }
        }
    }

    private void createSampleFile(String filename) throws IOException {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filename))) {
            writer.write("Это пример текста для лабораторной работы.");
            writer.newLine();
            writer.write("Текст содержит несколько предложений с разными разделителями.");
            writer.newLine();
            writer.write("Программа должна выровнять этот текст по заданной ширине.");
        }
        System.out.println("Создан файл " + filename + " с примером текста.");
    }
}

public class lab5_2 {
    public static void main(String[] args) {
        try {
            Scanner scanner = new Scanner(System.in);
            System.out.print("Введите ширину текста: ");
            int width = scanner.nextInt();

            if (width <= 0) {
                System.out.println("Ширина должна быть положительным числом!");
                return;
            }

            FileProcessor fileProcessor = new FileProcessor();
            TextFormatter formatter = new TextFormatter(width);

            String inputText = fileProcessor.readFile("input.txt");
            System.out.println("Прочитан текст: " + inputText);

            List<String> formattedText = formatter.formatText(inputText);

            fileProcessor.writeFile("output.txt", formattedText);

            System.out.println("Текст успешно отформатирован и сохранен в output.txt");

            // Вывод результата в консоль для проверки
            System.out.println("\nРезультат форматирования (ширина " + width + "):");
            System.out.println("┌" + "─".repeat(width) + "┐");
            for (String line : formattedText) {
                System.out.println("│" + line + "│");
            }
            System.out.println("└" + "─".repeat(width) + "┘");

            // Проверка длины строк
            System.out.println("\nПроверка длин строк:");
            boolean allCorrect = true;
            for (int i = 0; i < formattedText.size(); i++) {
                String line = formattedText.get(i);
                int lineLength = line.length();
                System.out.println("Строка " + (i + 1) + ": " + lineLength + " символов: '" + line + "'");
                if (lineLength != width) {
                    allCorrect = false;
                }
            }

            if (allCorrect) {
                System.out.println("✓ Все строки имеют правильную длину!");
            } else {
                System.out.println("⚠ Некоторые строки имеют неправильную длину");
            }

        } catch (IOException e) {
            System.err.println("Ошибка при работе с файлами: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("Произошла ошибка: " + e.getMessage());
        }
    }
}