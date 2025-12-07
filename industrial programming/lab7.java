import javax.swing.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

public class lab7 extends JFrame {
    private JTable table;
    private DefaultTableModel model;
    private JScrollPane scrollPane;

    // Данные для таблицы
    private Object[][] data = {
            {"Алексей Иванов", 8, "1 группа"},
            {"Мария Петрова", 10, "2 группа"},
            {"Иван Сидоров", 7, "1 группа"},
            {"Ольга Николаева", 9, "3 группа"},
            {"Дмитрий Кузнецов", 6, "2 группа"},
            {"Анна Смирнова", 10, "1 группа"},
            {"Сергей Васильев", 5, "3 группа"},
            {"Екатерина Морозова", 8, "2 группа"},
            {"Михаил Павлов", 7, "1 группа"},
            {"Наталья Федорова", 9, "3 группа"}
    };
    private String[] columnNames = {"Имя", "Оценка", "Группа"};

    public lab7() {
        setTitle("Таблица студентов");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        // Создаем модель таблицы
        model = new DefaultTableModel(data, columnNames) {
            @Override
            public Class<?> getColumnClass(int column) {
                switch (column) {
                    case 0: return String.class;
                    case 1: return Integer.class;
                    case 2: return String.class;
                    default: return Object.class;
                }
            }
        };

        table = new JTable(model);
        scrollPane = new JScrollPane(table);
        add(scrollPane, BorderLayout.CENTER);
        DefaultTableCellRenderer centerRenderer = new DefaultTableCellRenderer();
        centerRenderer.setHorizontalAlignment(JLabel.CENTER);

        // Применяем выравнивание ко всем столбцам
        for (int i = 0; i < table.getColumnCount(); i++) {
            table.getColumnModel().getColumn(i).setCellRenderer(centerRenderer);
        }
        // Панель для кнопок
        JPanel buttonPanel = new JPanel(new GridLayout(1, 6));

        // Создаем кнопки для сортировки
        String[] columns = {"Имя", "Оценка", "Группа"};
        for (int i = 0; i < columns.length; i++) {
            final int columnIndex = i;

            JButton ascButton = new JButton(columns[i] + " ↑");
            ascButton.addActionListener(e -> sortTable(columnIndex, true));

            JButton descButton = new JButton(columns[i] + " ↓");
            descButton.addActionListener(e -> sortTable(columnIndex, false));

            buttonPanel.add(ascButton);
            buttonPanel.add(descButton);
        }

        add(buttonPanel, BorderLayout.NORTH);

        setSize(600, 400);
        setLocationRelativeTo(null);
    }

    private void sortTable(int columnIndex, boolean ascending) {
        TableRowSorter<DefaultTableModel> sorter = new TableRowSorter<>(model);
        table.setRowSorter(sorter);

        ArrayList<RowSorter.SortKey> sortKeys = new ArrayList<>();

        // Определяем порядок сортировки
        SortOrder sortOrder = ascending ? SortOrder.ASCENDING : SortOrder.DESCENDING;
        sortKeys.add(new RowSorter.SortKey(columnIndex, sortOrder));

        sorter.setSortKeys(sortKeys);
        sorter.sort();
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            new lab7().setVisible(true);
        });
    }
}