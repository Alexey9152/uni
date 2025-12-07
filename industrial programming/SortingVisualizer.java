import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

public class SortingVisualizer extends JFrame {
    private static final int ARRAY_SIZE = 25;
    private static final int MAX_VAL = 100;
    private SortingPanel[] panels = new SortingPanel[3];
    private int[] base = genRandomArray(ARRAY_SIZE);

    public SortingVisualizer() {
        setTitle("Визуализация сортировок");
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        // Верхняя панель с кнопками
        JPanel ctrlPanel = new JPanel();
        JButton fileBtn = new JButton("Выбрать файл");
        JButton sortBtn = new JButton("Запустить");

        ctrlPanel.add(fileBtn);
        ctrlPanel.add(sortBtn);
        add(ctrlPanel, BorderLayout.NORTH);

        // Три визуализатора
        JPanel columns = new JPanel(new GridLayout(1, 3, 10, 0));
        panels[0] = new SortingPanel("Пузырь", base.clone());
        panels[1] = new SortingPanel("Выбор", base.clone());
        panels[2] = new SortingPanel("Вставки", base.clone());
        for (SortingPanel sp : panels) columns.add(sp);
        add(columns, BorderLayout.CENTER);

        // Чтение массива из файла
        fileBtn.addActionListener(e -> {
            JFileChooser chooser = new JFileChooser();
            if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                File f = chooser.getSelectedFile();
                try (Scanner sc = new Scanner(f)) {
                    java.util.List<Integer> lst = new java.util.ArrayList<>();
                    while (sc.hasNextInt()) lst.add(sc.nextInt());
                    if (!lst.isEmpty()) {
                        base = lst.stream().mapToInt(i -> i).toArray();
                        for(int i=0; i<3; ++i) panels[i].setArray(base.clone());
                    }
                } catch (Exception ex) {
                    JOptionPane.showMessageDialog(this, "Ошибка чтения файла", "", JOptionPane.ERROR_MESSAGE);
                }
            }
        });

        // Запуск сортировок
        sortBtn.addActionListener(e -> {
            for (SortingPanel panel : panels)
                panel.startSortInThread();
        });

        setSize(900, 600);
        setLocationRelativeTo(null);
        setVisible(true);
    }

    private static int[] genRandomArray(int n) {
        Random rnd = new Random();
        int[] arr = new int[n];
        for (int i = 0; i < n; i++) arr[i] = rnd.nextInt(MAX_VAL) + 1;
        return arr;
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(SortingVisualizer::new);
    }

    // Один столбик-сортировщик
    static class SortingPanel extends JPanel {
        private int[] arr;
        private final String title;
        private volatile boolean sorting = false;

        SortingPanel(String title, int[] arr) {
            this.title = title;
            this.arr = arr;
            setBackground(Color.WHITE);
        }

        void setArray(int[] arr) {
            this.arr = arr;
            sorting = false;
            repaint();
        }

        void startSortInThread() {
            if (sorting) return;
            sorting = true;
            int[] a = arr.clone();
            new Thread(() -> {
                switch (title) {
                    case "Пузырь" -> bubble(a);
                    case "Выбор" -> selection(a);
                    case "Вставки" -> insertion(a);
                }
                sorting = false;
            }).start();
        }

        private void bubble(int[] a) {
            for (int i = 0; i < a.length - 1; i++)
                for (int j = 0; j < a.length - 1 - i; j++)
                    if (a[j] > a[j + 1]) { swap(a,j,j+1); sleep(); }
            arr = a; repaint();
        }
        private void selection(int[] a) {
            for (int i = 0; i < a.length - 1; i++) {
                int min = i;
                for (int j = i + 1; j < a.length; j++)
                    if (a[j] < a[min]) min = j;
                if (min != i) { swap(a, i, min); sleep(); }
            }
            arr = a; repaint();
        }
        private void insertion(int[] a) {
            for (int i = 1; i < a.length; i++) {
                int key = a[i], j = i - 1;
                while (j >= 0 && a[j] > key) {
                    a[j + 1] = a[j];
                    j--;
                    arr = a.clone(); repaint(); sleep();
                }
                a[j + 1] = key;
                arr = a.clone(); repaint(); sleep();
            }
            arr = a; repaint();
        }
        private void swap(int[] a, int i, int j) {
            int t = a[i]; a[i] = a[j]; a[j] = t;
            arr = a.clone(); repaint();
        }
        private void sleep() {
            try { Thread.sleep(60); } catch (InterruptedException ignored) {}
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            int w = getWidth(), h = getHeight();
            int n = arr.length;
            int barHeight = h/(n+2);
            int max = Arrays.stream(arr).max().orElse(1);

            g.setColor(Color.BLACK);
            g.drawString(title, w / 2 - 30, 20);

            for (int i = 0; i < n; ++i) {
                int val = arr[i];
                int barLen = (int)((double)val / max * (w - 60));
                int x = 30, y = 35 + i * barHeight;
                g.setColor(new Color(30,120,240));
                g.fillRect(x, y, barLen, barHeight-5);
                g.setColor(Color.DARK_GRAY);
                g.drawRect(x, y, barLen, barHeight-5);
            }
        }
    }
}
