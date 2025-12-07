import org.w3c.dom.*;
import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

class Book {
    private String id;
    private String title;
    private String authors; // "Автор1, Автор2"
    private int year;
    private String category;
    private double price;
    private int total;
    private int available;

    public Book(String id, String title, String authors,
                int year, String category, double price,
                int total, int available) {
        this.id = id;
        this.title = title;
        this.authors = authors;
        this.year = year;
        this.category = category;
        this.price = price;
        this.total = total;
        this.available = available;
    }

    // геттеры и сеттеры
    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getTitle() { return title; }
    public void setTitle(String title) { this.title = title; }

    public String getAuthors() { return authors; }
    public void setAuthors(String authors) { this.authors = authors; }

    public int getYear() { return year; }
    public void setYear(int year) { this.year = year; }

    public String getCategory() { return category; }
    public void setCategory(String category) { this.category = category; }

    public double getPrice() { return price; }
    public void setPrice(double price) { this.price = price; }

    public int getTotal() { return total; }
    public void setTotal(int total) { this.total = total; }

    public int getAvailable() { return available; }
    public void setAvailable(int available) { this.available = available; }
}


public class LibraryApp extends JFrame {

    private List<Book> books = new ArrayList<>();
    private BooksTableModel tableModel = new BooksTableModel();
    private JTable table;

    public LibraryApp() {
        super("Библиотека (XML)");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(900, 500);
        setLocationRelativeTo(null);

        table = new JTable(tableModel);
        JScrollPane scrollPane = new JScrollPane(table);

        JButton btnLoad = new JButton("Загрузить XML");
        JButton btnAdd = new JButton("Добавить книгу");
        JButton btnSearch = new JButton("Поиск");
        JButton btnReprice = new JButton("Переоценка");
        JButton btnIssue = new JButton("Выдать книгу");

        btnLoad.addActionListener(this::onLoad);
        btnAdd.addActionListener(this::onAdd);
        btnSearch.addActionListener(this::onSearch);
        btnReprice.addActionListener(this::onReprice);
        btnIssue.addActionListener(this::onIssue);

        JPanel buttons = new JPanel();
        buttons.add(btnLoad);
        buttons.add(btnAdd);
        buttons.add(btnSearch);
        buttons.add(btnReprice);
        buttons.add(btnIssue);

        add(scrollPane, BorderLayout.CENTER);
        add(buttons, BorderLayout.SOUTH);
    }

    private void onLoad(ActionEvent e) {
        JFileChooser chooser = new JFileChooser();
        chooser.setDialogTitle("Выберите library.xml");
        if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            File xmlFile = chooser.getSelectedFile();
            File xsdFile = new File(xmlFile.getParentFile(), "library.xsd");
            try {
                validateXml(xmlFile, xsdFile);
                loadFromXml(xmlFile);
                JOptionPane.showMessageDialog(this, "XML загружен и валиден.");
            } catch (Exception ex) {
                ex.printStackTrace();
                JOptionPane.showMessageDialog(this,
                        "Ошибка при загрузке/валидации: " + ex.getMessage(),
                        "Ошибка", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    private void validateXml(File xml, File xsd) throws Exception {
        SchemaFactory factory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
        Schema schema = factory.newSchema(xsd);
        Validator validator = schema.newValidator();
        validator.validate(new StreamSource(xml));
    }

    private void loadFromXml(File xmlFile) throws Exception {
        books.clear();

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        DocumentBuilder builder = factory.newDocumentBuilder();
        Document doc = builder.parse(xmlFile);

        NodeList bookNodes = doc.getElementsByTagName("book");
        for (int i = 0; i < bookNodes.getLength(); i++) {
            Element b = (Element) bookNodes.item(i);
            String id = b.getAttribute("id");
            int total = Integer.parseInt(b.getAttribute("total"));
            int available = Integer.parseInt(b.getAttribute("available"));

            String title = b.getElementsByTagName("title").item(0).getTextContent();

            NodeList authorsNodes = b.getElementsByTagName("author");
            StringBuilder authorsSb = new StringBuilder();
            for (int j = 0; j < authorsNodes.getLength(); j++) {
                if (j > 0) authorsSb.append(", ");
                authorsSb.append(authorsNodes.item(j).getTextContent());
            }

            int year = Integer.parseInt(b.getElementsByTagName("year").item(0).getTextContent());
            String category = b.getElementsByTagName("category").item(0).getTextContent();
            double price = Double.parseDouble(b.getElementsByTagName("price").item(0).getTextContent());

            books.add(new Book(id, title, authorsSb.toString(), year, category, price, total, available));
        }
        tableModel.fireTableDataChanged();
    }

    private void onAdd(ActionEvent e) {
        JTextField idField = new JTextField();
        JTextField titleField = new JTextField();
        JTextField authorsField = new JTextField();
        JTextField yearField = new JTextField();
        JTextField categoryField = new JTextField();
        JTextField priceField = new JTextField();
        JTextField totalField = new JTextField();
        JTextField availableField = new JTextField();

        JPanel panel = new JPanel(new GridLayout(0, 2));
        panel.add(new JLabel("ID:"));
        panel.add(idField);
        panel.add(new JLabel("Название:"));
        panel.add(titleField);
        panel.add(new JLabel("Авторы (через ,):"));
        panel.add(authorsField);
        panel.add(new JLabel("Год:"));
        panel.add(yearField);
        panel.add(new JLabel("Категория:"));
        panel.add(categoryField);
        panel.add(new JLabel("Цена:"));
        panel.add(priceField);
        panel.add(new JLabel("Всего экземпляров:"));
        panel.add(totalField);
        panel.add(new JLabel("В наличии:"));
        panel.add(availableField);

        int result = JOptionPane.showConfirmDialog(this, panel,
                "Добавление книги", JOptionPane.OK_CANCEL_OPTION);
        if (result == JOptionPane.OK_OPTION) {
            try {
                String id = idField.getText().trim();
                String title = titleField.getText().trim();
                String authors = authorsField.getText().trim();
                int year = Integer.parseInt(yearField.getText().trim());
                String category = categoryField.getText().trim();
                double price = Double.parseDouble(priceField.getText().trim());
                int total = Integer.parseInt(totalField.getText().trim());
                int available = Integer.parseInt(availableField.getText().trim());
                if (available > total) {
                    JOptionPane.showMessageDialog(this, "В наличии не может быть больше, чем всего.");
                    return;
                }
                books.add(new Book(id, title, authors, year, category, price, total, available));
                tableModel.fireTableDataChanged();
            } catch (NumberFormatException ex) {
                JOptionPane.showMessageDialog(this, "Ошибка формата чисел.");
            }
        }
    }

    private void onSearch(ActionEvent e) {
        String[] options = {"Автор", "Год", "Категория"};
        String field = (String) JOptionPane.showInputDialog(this,
                "Поле поиска:", "Поиск",
                JOptionPane.QUESTION_MESSAGE, null, options, options[0]);
        if (field == null) return;

        String query = JOptionPane.showInputDialog(this, "Введите значение для поиска:");
        if (query == null || query.isEmpty()) return;

        query = query.toLowerCase();
        List<Book> found = new ArrayList<>();
        for (Book b : books) {
            switch (field) {
                case "Автор":
                    if (b.getAuthors().toLowerCase().contains(query)) found.add(b);
                    break;
                case "Год":
                    if (String.valueOf(b.getYear()).equals(query)) found.add(b);
                    break;
                case "Категория":
                    if (b.getCategory().toLowerCase().contains(query)) found.add(b);
                    break;
            }
        }

        if (found.isEmpty()) {
            JOptionPane.showMessageDialog(this, "Ничего не найдено.");
        } else {
            StringBuilder sb = new StringBuilder();
            for (Book b : found) {
                sb.append(String.format("ID=%s, %s (%d), %s, %.2f%n",
                        b.getId(), b.getTitle(), b.getYear(),
                        b.getCategory(), b.getPrice()));
            }
            JOptionPane.showMessageDialog(this, sb.toString(), "Результаты", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    private void onReprice(ActionEvent e) {
        int row = table.getSelectedRow();
        if (row < 0) {
            JOptionPane.showMessageDialog(this, "Выберите книгу в таблице.");
            return;
        }
        Book b = books.get(row);
        String s = JOptionPane.showInputDialog(this,
                "Новая цена для \"" + b.getTitle() + "\":", b.getPrice());
        if (s == null) return;
        try {
            double newPrice = Double.parseDouble(s);
            b.setPrice(newPrice);
            tableModel.fireTableRowsUpdated(row, row);
        } catch (NumberFormatException ex) {
            JOptionPane.showMessageDialog(this, "Некорректная цена.");
        }
    }

    private void onIssue(ActionEvent e) {
        int row = table.getSelectedRow();
        if (row < 0) {
            JOptionPane.showMessageDialog(this, "Выберите книгу в таблице.");
            return;
        }
        Book b = books.get(row);
        if (b.getAvailable() <= 0) {
            JOptionPane.showMessageDialog(this, "Нет доступных экземпляров для выдачи.");
            return;
        }
        b.setAvailable(b.getAvailable() - 1);
        tableModel.fireTableRowsUpdated(row, row);
    }

    private class BooksTableModel extends AbstractTableModel {
        private final String[] columns = {
                "ID", "Название", "Авторы", "Год",
                "Категория", "Цена", "Всего", "В наличии"
        };

        @Override
        public int getRowCount() {
            return books.size();
        }

        @Override
        public int getColumnCount() {
            return columns.length;
        }

        @Override
        public String getColumnName(int column) {
            return columns[column];
        }

        @Override
        public Object getValueAt(int rowIndex, int columnIndex) {
            Book b = books.get(rowIndex);
            switch (columnIndex) {
                case 0: return b.getId();
                case 1: return b.getTitle();
                case 2: return b.getAuthors();
                case 3: return b.getYear();
                case 4: return b.getCategory();
                case 5: return b.getPrice();
                case 6: return b.getTotal();
                case 7: return b.getAvailable();
            }
            return null;
        }

        @Override
        public boolean isCellEditable(int rowIndex, int columnIndex) {
            return false;
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new LibraryApp().setVisible(true));
    }
}
