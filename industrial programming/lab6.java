import javax.swing.*;
import javax.swing.border.TitledBorder;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import java.util.List;

public class lab6 extends JFrame {
    private JTextField expressionField;
    private JTextField variablesField;
    private JTextField rpnField;
    private JTextField resultField;
    private JButton calculateButton;

    public lab6() {
        setTitle("Калькулятор выражений");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout(10, 10));

        // Основная панель с отступами
        JPanel mainPanel = new JPanel(new GridLayout(4, 1, 10, 10));
        mainPanel.setBorder(BorderFactory.createEmptyBorder(15, 15, 15, 15));

        // Панель для ввода выражения
        JPanel expressionPanel = new JPanel(new BorderLayout(5, 5));
        expressionPanel.setBorder(new TitledBorder("Введите выражение (поддерживаются +, -, *, /, (, ) и переменные)"));
        expressionField = new JTextField();
        expressionField.setFont(new Font("Monospaced", Font.PLAIN, 14));
        expressionPanel.add(expressionField, BorderLayout.CENTER);

        // Панель для ввода переменных
        JPanel variablesPanel = new JPanel(new BorderLayout(5, 5));
        variablesPanel.setBorder(new TitledBorder("Введите переменные (формат: a=2, b=3, c=5)"));
        variablesField = new JTextField();
        variablesField.setFont(new Font("Monospaced", Font.PLAIN, 14));
        variablesPanel.add(variablesField, BorderLayout.CENTER);

        // Панель для польской записи
        JPanel rpnPanel = new JPanel(new BorderLayout(5, 5));
        rpnPanel.setBorder(new TitledBorder("Польская запись (RPN)"));
        rpnField = new JTextField();
        rpnField.setFont(new Font("Monospaced", Font.BOLD, 14));
        rpnField.setEditable(false);
        rpnField.setBackground(new Color(240, 240, 240));
        rpnPanel.add(rpnField, BorderLayout.CENTER);

        // Панель для результата
        JPanel resultPanel = new JPanel(new BorderLayout(5, 5));
        resultPanel.setBorder(new TitledBorder("Результат"));
        resultField = new JTextField();
        resultField.setFont(new Font("Monospaced", Font.BOLD, 16));
        resultField.setEditable(false);
        resultField.setBackground(new Color(220, 240, 255));
        resultPanel.add(resultField, BorderLayout.CENTER);

        // Кнопка вычисления
        calculateButton = new JButton("Вычислить");
        calculateButton.setFont(new Font("Arial", Font.BOLD, 14));
        calculateButton.setBackground(new Color(70, 130, 180));
        calculateButton.setForeground(Color.WHITE);

        // Добавляем компоненты
        mainPanel.add(expressionPanel);
        mainPanel.add(variablesPanel);
        mainPanel.add(rpnPanel);
        mainPanel.add(resultPanel);

        add(mainPanel, BorderLayout.CENTER);
        add(calculateButton, BorderLayout.SOUTH);

        // Обработчик кнопки
        calculateButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                calculateExpression();
            }
        });

        // Размер и отображение
        setSize(700, 450);
        setLocationRelativeTo(null);
    }

    private void calculateExpression() {
        try {
            String expression = expressionField.getText().trim();
            String variablesText = variablesField.getText().trim();

            // Валидация ввода
            if (expression.isEmpty()) {
                showError("Введите математическое выражение!");
                return;
            }

            // Проверка на недопустимые символы
            if (!isValidExpression(expression)) {
                showError("Выражение содержит недопустимые символы!\n" +
                        "Разрешены: буквы, цифры, операторы + - * / и скобки ( )");
                return;
            }

            // Парсим переменные
            Map<String, Double> variables = parseVariables(variablesText);

            // Проверяем, что все переменные в выражении определены
            List<String> undefinedVariables = findUndefinedVariables(expression, variables);
            if (!undefinedVariables.isEmpty()) {
                showError("Не определены переменные: " + undefinedVariables +
                        "\nВведите их значения в поле для переменных.");
                return;
            }

            // Проверяем баланс скобок
            if (!checkParenthesesBalance(expression)) {
                showError("Несбалансированные скобки в выражении!");
                return;
            }

            // Преобразуем в польскую запись
            List<String> rpnTokens = infixToRPN(expression, variables);
            String rpn = String.join(" ", rpnTokens);
            rpnField.setText(rpn);

            // Вычисляем результат
            double result = evaluateRPN(rpnTokens, variables);
            resultField.setText(String.format("%.6f", result));

        } catch (NumberFormatException ex) {
            showError("Некорректный формат числа!\nПроверьте значения переменных.");
        } catch (ArithmeticException ex) {
            showError("Арифметическая ошибка: " + ex.getMessage());
        } catch (EmptyStackException ex) {
            showError("Некорректное выражение: недостаточно операндов для операторов");
        } catch (Exception ex) {
            showError("Ошибка вычисления: " + ex.getMessage());
        }
    }

    private boolean isValidExpression(String expression) {
        // Разрешаем буквы, цифры, операторы, скобки и пробелы
        return expression.matches("^[a-zA-Zа-яА-Я0-9+\\-*/()., \\s]+$");
    }

    private List<String> findUndefinedVariables(String expression, Map<String, Double> variables) {
        List<String> undefined = new ArrayList<>();
        Set<String> usedVariables = extractVariables(expression);

        for (String var : usedVariables) {
            if (!variables.containsKey(var) && !isNumber(var)) {
                undefined.add(var);
            }
        }

        return undefined;
    }

    private Set<String> extractVariables(String expression) {
        Set<String> variables = new HashSet<>();
        String[] tokens = expression.split("[+\\-*/()\\s]+");

        for (String token : tokens) {
            if (!token.isEmpty() && !isNumber(token) && token.matches("[a-zA-Zа-яА-Я]+")) {
                variables.add(token);
            }
        }

        return variables;
    }

    private boolean checkParenthesesBalance(String expression) {
        int balance = 0;
        for (char c : expression.toCharArray()) {
            if (c == '(') balance++;
            if (c == ')') balance--;
            if (balance < 0) return false; // Закрывающая скобка без открывающей
        }
        return balance == 0;
    }

    private Map<String, Double> parseVariables(String variablesText) {
        Map<String, Double> variables = new HashMap<>();

        if (variablesText.isEmpty()) {
            return variables;
        }

        try {
            // Разделяем по запятым
            String[] pairs = variablesText.split(",");
            for (String pair : pairs) {
                pair = pair.trim();
                if (!pair.isEmpty()) {
                    String[] parts = pair.split("=");
                    if (parts.length != 2) {
                        throw new IllegalArgumentException("Некорректный формат переменной: " + pair +
                                "\nИспользуйте формат: имя=значение");
                    }

                    String name = parts[0].trim();
                    String valueStr = parts[1].trim();

                    // Проверяем имя переменной
                    if (!name.matches("[a-zA-Zа-яА-Я]+")) {
                        throw new IllegalArgumentException("Некорректное имя переменной: " + name +
                                "\nИмя должно содержать только буквы");
                    }

                    // Парсим значение
                    double value = Double.parseDouble(valueStr);
                    variables.put(name, value);
                }
            }
        } catch (NumberFormatException ex) {
            throw new NumberFormatException("Некорректное числовое значение в переменных!");
        }

        return variables;
    }

    private List<String> infixToRPN(String expression, Map<String, Double> variables) {
        List<String> output = new ArrayList<>();
        Stack<String> stack = new Stack<>();

        // Удаляем пробелы и добавляем пробелы вокруг операторов для токенизации
        expression = expression.replaceAll("\\s+", "");
        expression = expression.replaceAll("([()+\\-*/])", " $1 ");
        expression = expression.trim().replaceAll("\\s+", " ");

        String[] tokens = expression.split(" ");

        Map<String, Integer> precedence = new HashMap<>();
        precedence.put("+", 1);
        precedence.put("-", 1);
        precedence.put("*", 2);
        precedence.put("/", 2);
        precedence.put("(", 0);

        for (String token : tokens) {
            if (token.isEmpty()) continue;

            if (isNumber(token) || variables.containsKey(token) || token.matches("[a-zA-Zа-яА-Я]+")) {
                // Число или переменная
                output.add(token);
            } else if (token.equals("(")) {
                stack.push(token);
            } else if (token.equals(")")) {
                while (!stack.isEmpty() && !stack.peek().equals("(")) {
                    output.add(stack.pop());
                }
                if (stack.isEmpty()) {
                    throw new RuntimeException("Несбалансированные скобки");
                }
                stack.pop(); // Удаляем "("
            } else if (isOperator(token)) {
                // Оператор
                while (!stack.isEmpty() && precedence.getOrDefault(stack.peek(), 0) >= precedence.get(token)) {
                    output.add(stack.pop());
                }
                stack.push(token);
            } else {
                throw new RuntimeException("Неизвестный токен: " + token);
            }
        }

        while (!stack.isEmpty()) {
            String operator = stack.pop();
            if (operator.equals("(")) {
                throw new RuntimeException("Несбалансированные скобки");
            }
            output.add(operator);
        }

        return output;
    }

    private double evaluateRPN(List<String> rpnTokens, Map<String, Double> variables) {
        Stack<Double> stack = new Stack<>();

        for (String token : rpnTokens) {
            if (isNumber(token)) {
                stack.push(Double.parseDouble(token));
            } else if (variables.containsKey(token)) {
                stack.push(variables.get(token));
            } else if (isOperator(token)) {
                if (stack.size() < 2) {
                    throw new RuntimeException("Недостаточно операндов для оператора: " + token);
                }
                double b = stack.pop();
                double a = stack.pop();
                double result = applyOperator(token, a, b);
                stack.push(result);
            } else {
                throw new RuntimeException("Неизвестный токен в RPN: " + token);
            }
        }

        if (stack.size() != 1) {
            throw new RuntimeException("Некорректное выражение: в стеке осталось " + stack.size() + " значений");
        }

        return stack.pop();
    }

    private boolean isNumber(String str) {
        try {
            Double.parseDouble(str);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }

    private boolean isOperator(String str) {
        return str.equals("+") || str.equals("-") || str.equals("*") || str.equals("/");
    }

    private double applyOperator(String operator, double a, double b) {
        switch (operator) {
            case "+": return a + b;
            case "-": return a - b;
            case "*": return a * b;
            case "/":
                if (b == 0) throw new ArithmeticException("Деление на ноль");
                return a / b;
            default: throw new IllegalArgumentException("Неизвестный оператор: " + operator);
        }
    }

    private void showError(String message) {
        JOptionPane.showMessageDialog(this, message, "Ошибка", JOptionPane.ERROR_MESSAGE);
        rpnField.setText("");
        resultField.setText("");
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            new lab6().setVisible(true);
        });
    }
}