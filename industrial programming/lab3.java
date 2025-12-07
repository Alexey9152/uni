import java.io.*;
import java.util.StringTokenizer;
import java.nio.charset.StandardCharsets;

public class lab3 {

    public static void main(String[] args) {
        // Явно указываем кодировку UTF-8 для чтения из консоли
        InputStreamReader isr = new InputStreamReader(System.in, StandardCharsets.UTF_8);
        BufferedReader br = new BufferedReader(isr);

        try {
            System.out.println("Введите признак (0 или 1), символ и при необходимости k:");
            String paramsLine = br.readLine();
            StringTokenizer paramsTokenizer = new StringTokenizer(paramsLine);

            if (paramsTokenizer.countTokens() < 2) {
                System.out.println("Ошибка: недостаточно параметров.");
                return;
            }

            int action = Integer.parseInt(paramsTokenizer.nextToken());
            char symbol = paramsTokenizer.nextToken().charAt(0);
            int k = 0;

            if (action == 1) {
                if (paramsTokenizer.hasMoreTokens()) {
                    k = Integer.parseInt(paramsTokenizer.nextToken());
                } else {
                    System.out.println("Ошибка: для признака 1 необходимо указать k.");
                    return;
                }
            }

            System.out.println("Введите строки текста (пустая строка для завершения):");
            String line;
            while ((line = br.readLine()) != null && !line.isEmpty()) {
                if (action == 0) {
                    line = line.replace(Character.toString(symbol), "");
                } else {
                    if (k >= 0 && k <= line.length()) {
                        StringBuilder sb = new StringBuilder(line);
                        sb.insert(k, symbol);
                        line = sb.toString();
                    }
                }
                System.out.println(line);
            }
        } catch (IOException e) {
            System.out.println("Ошибка чтения с клавиатуры.");
        } catch (NumberFormatException e) {
            System.out.println("Ошибка: неверный формат числа.");
        }
    }
}