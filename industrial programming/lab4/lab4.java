import java.io.*;
import java.util.*;

// Класс для представления экзамена
class Exam {
    private String subject;
    private int grade;

    public Exam(String subject, int grade) {
        this.subject = subject;
        this.grade = grade;
    }

    public String getSubject() {
        return subject;
    }

    public int getGrade() {
        return grade;
    }
}

// Класс для представления зачета
class Test {
    private String subject;
    private boolean passed;

    public Test(String subject, boolean passed) {
        this.subject = subject;
        this.passed = passed;
    }

    public String getSubject() {
        return subject;
    }

    public boolean isPassed() {
        return passed;
    }
}

// Класс Зачетная книжка с внутренним классом Session
class RecordBook {
    private List<Session> sessions;

    public RecordBook() {
        sessions = new ArrayList<>();
    }

    public void addSession(Session session) {
        sessions.add(session);
    }

    public List<Session> getSessions() {
        return sessions;
    }

    // Метод для расчета среднего балла по всем сессиям
    public double calculateAverageGrade() {
        int totalGrade = 0;
        int examCount = 0;

        for (Session session : sessions) {
            for (Exam exam : session.getExams()) {
                totalGrade += exam.getGrade();
                examCount++;
            }
        }

        return examCount > 0 ? (double) totalGrade / examCount : 0.0;
    }

    // Внутренний класс Session
    public class Session {
        private int sessionNumber;
        private List<Exam> exams;
        private List<Test> tests;

        public Session(int sessionNumber) {
            this.sessionNumber = sessionNumber;
            exams = new ArrayList<>();
            tests = new ArrayList<>();
        }

        public void addExam(Exam exam) {
            exams.add(exam);
        }

        public void addTest(Test test) {
            tests.add(test);
        }

        public int getSessionNumber() {
            return sessionNumber;
        }

        public List<Exam> getExams() {
            return exams;
        }

        public List<Test> getTests() {
            return tests;
        }

        // Метод для расчета среднего балла за сессию
        public double getSessionAverage() {
            if (exams.isEmpty()) return 0.0;

            int total = 0;
            for (Exam exam : exams) {
                total += exam.getGrade();
            }
            return (double) total / exams.size();
        }
    }
}

// Класс Student для представления студента
class Student {
    private String lastName;
    private String firstName;
    private String patronymic;
    private String course;
    private String group;
    private RecordBook recordBook;

    public Student(String lastName, String firstName, String patronymic, String course, String group) {
        this.lastName = lastName;
        this.firstName = firstName;
        this.patronymic = patronymic;
        this.course = course;
        this.group = group;
        this.recordBook = new RecordBook();
    }

    public String getLastName() {
        return lastName;
    }

    public String getFirstName() {
        return firstName;
    }

    public String getPatronymic() {
        return patronymic;
    }

    public String getCourse() {
        return course;
    }

    public String getGroup() {
        return group;
    }

    public RecordBook getRecordBook() {
        return recordBook;
    }

    // Метод для получения полного имени
    public String getFullName() {
        return lastName + " " + firstName + " " + patronymic;
    }
}

public class lab4 {
    public static void main(String[] args) {
        List<Student> students = new ArrayList<>();

        // Чтение данных из файла
        try (BufferedReader reader = new BufferedReader(new FileReader("input.txt"))) {
            String line;
            Student currentStudent = null;
            RecordBook.Session currentSession = null;

            while ((line = reader.readLine()) != null) {
                if (line.trim().isEmpty()) {
                    continue;
                }

                // Проверяем, является ли строка информацией о студенте
                if (!line.startsWith("Сессия") && !line.startsWith("Экзамены:") &&
                        !line.startsWith("Зачеты:") && (currentStudent == null || currentSession != null)) {

                    String[] tokens = line.split("\\s+");
                    if (tokens.length >= 5) {
                        String lastName = tokens[0];
                        String firstName = tokens[1];
                        String patronymic = tokens[2];
                        String course = tokens[3];

                        // Собираем группу (она может содержать пробелы)
                        StringBuilder groupBuilder = new StringBuilder();
                        for (int i = 4; i < tokens.length; i++) {
                            groupBuilder.append(tokens[i]);
                            if (i < tokens.length - 1) {
                                groupBuilder.append(" ");
                            }
                        }
                        String group = groupBuilder.toString();

                        currentStudent = new Student(lastName, firstName, patronymic, course, group);
                        students.add(currentStudent);
                        currentSession = null; // Сбрасываем текущую сессию
                    }
                }
                // Обрабатываем информацию о сессии
                else if (line.startsWith("Сессия") && currentStudent != null) {
                    String[] sessionTokens = line.split("\\s+");
                    if (sessionTokens.length >= 2) {
                        try {
                            int sessionNumber = Integer.parseInt(sessionTokens[1]);
                            currentSession = currentStudent.getRecordBook().new Session(sessionNumber);
                            currentStudent.getRecordBook().addSession(currentSession);
                        } catch (NumberFormatException e) {
                            // Игнорируем ошибку парсинга
                        }
                    }
                }
                // Обрабатываем экзамены
                else if (line.equals("Экзамены:") && currentSession != null) {
                    // Читаем экзамены до строки "Зачеты:" или пустой строки
                    line = reader.readLine();
                    while (line != null && !line.trim().isEmpty() && !line.equals("Зачеты:")) {
                        String[] examTokens = line.split("\\s+");
                        if (examTokens.length >= 2) {
                            try {
                                int grade = Integer.parseInt(examTokens[examTokens.length - 1]);
                                StringBuilder subjectBuilder = new StringBuilder();
                                for (int i = 0; i < examTokens.length - 1; i++) {
                                    subjectBuilder.append(examTokens[i]);
                                    if (i < examTokens.length - 2) {
                                        subjectBuilder.append(" ");
                                    }
                                }
                                String subject = subjectBuilder.toString();
                                currentSession.addExam(new Exam(subject, grade));
                            } catch (NumberFormatException e) {
                                // Игнорируем строки с некорректными оценками
                            }
                        }
                        line = reader.readLine();
                    }
                    // Если следующая строка - "Зачеты:", продолжаем обработку
                    if (line != null && line.equals("Зачеты:")) {
                        // Продолжим в следующем условии
                    } else {
                        continue;
                    }
                }
                // Обрабатываем зачеты
                if (line != null && line.equals("Зачеты:") && currentSession != null) {
                    line = reader.readLine();
                    while (line != null && !line.trim().isEmpty() &&
                            !line.startsWith("Сессия") &&
                            !(line.split("\\s+").length >= 5 && !line.startsWith("Сессия") &&
                                    !line.startsWith("Экзамены:") && !line.startsWith("Зачеты:"))) {

                        if (line.trim().isEmpty()) {
                            break;
                        }

                        String[] testTokens = line.split("\\s+");
                        if (testTokens.length >= 2) {
                            String status = testTokens[testTokens.length - 1];
                            boolean passed = status.equals("сдано");

                            StringBuilder subjectBuilder = new StringBuilder();
                            for (int i = 0; i < testTokens.length - 1; i++) {
                                subjectBuilder.append(testTokens[i]);
                                if (i < testTokens.length - 2) {
                                    subjectBuilder.append(" ");
                                }
                            }
                            String subject = subjectBuilder.toString();
                            currentSession.addTest(new Test(subject, passed));
                        }
                        line = reader.readLine();
                        if (line == null) break;
                    }
                }
            }
        } catch (IOException e) {
            System.out.println("Ошибка чтения файла: " + e.getMessage());
            e.printStackTrace();
        }

        // Запись результатов в файл
        try (PrintWriter writer = new PrintWriter(new FileWriter("output.txt"))) {
            writer.println("ОТЧЕТ ПО СТУДЕНТАМ");
            writer.println("=================");
            writer.println();

            for (Student student : students) {
                writer.println("СТУДЕНТ: " + student.getFullName());
                writer.println("Курс: " + student.getCourse() + ", Группа: " + student.getGroup());
                writer.println();

                RecordBook recordBook = student.getRecordBook();
                double overallAverage = recordBook.calculateAverageGrade();

                for (RecordBook.Session session : recordBook.getSessions()) {
                    writer.println("Сессия " + session.getSessionNumber() + ":");
                    writer.println("Экзамены:");

                    for (Exam exam : session.getExams()) {
                        writer.println("  - " + exam.getSubject() + ": " + exam.getGrade());
                    }

                    writer.println("Зачеты:");
                    for (Test test : session.getTests()) {
                        writer.println("  - " + test.getSubject() + ": " + (test.isPassed() ? "сдано" : "не сдано"));
                    }

                    double sessionAverage = session.getSessionAverage();
                    writer.printf("Средний балл за сессию: %.2f\n", sessionAverage);
                    writer.println();
                }

                writer.printf("ОБЩИЙ СРЕДНИЙ БАЛЛ: %.2f\n", overallAverage);
                writer.println("=========================================");
                writer.println();
            }

            // Добавляем общую статистику
            writer.println("ОБЩАЯ СТАТИСТИКА:");
            writer.println("Всего студентов: " + students.size());

            if (!students.isEmpty()) {
                double totalAverage = students.stream()
                        .mapToDouble(s -> s.getRecordBook().calculateAverageGrade())
                        .average()
                        .orElse(0.0);
                writer.printf("Средний балл по всем студентам: %.2f\n", totalAverage);
            }

        } catch (IOException e) {
            System.out.println("Ошибка записи файла: " + e.getMessage());
        }

        System.out.println("Обработка завершена. Результаты сохранены в output.txt");
        System.out.println("Обработано студентов: " + students.size());
    }
}