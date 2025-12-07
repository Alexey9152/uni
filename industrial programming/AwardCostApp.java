import javax.swing.*;
import javax.swing.border.TitledBorder;
import java.awt.*;
import java.util.*;
import java.util.List;

public class AwardCostApp extends JFrame {

    // Модель данных
    private static class GiftOption {
        final String name;
        final int price; // BYN
        GiftOption(String name, int price) { this.name = name; this.price = price; }
        @Override public String toString() { return name + " — " + price + " BYN"; }
    }

    // Конфигурация (BYN)
    private final int concertFixedCost = 500; // BYN (фиксированная стоимость концерта)
    private final List<String> greeters = List.of("Коммунарка", "Семь пятниц", "Apple", "Интеграл");
    private final Map<String, List<GiftOption>> giftsByGreeter = new HashMap<>();

    // UI
    private final Map<String, JCheckBox> greeterChecks = new LinkedHashMap<>();
    private final JList<GiftOption> giftList = new JList<>();
    private final DefaultListModel<GiftOption> giftListModel = new DefaultListModel<>();
    private final JRadioButton concertYes = new JRadioButton("Да");
    private final JRadioButton concertNo = new JRadioButton("Нет", true);
    private final JCheckBox loyalCustomer = new JCheckBox("Постоянный клиент (−10%)");
    private final JLabel totalLabel = new JLabel("Итого: 0 BYN");
    private final JTextArea orderText = new JTextArea(8, 60);

    public AwardCostApp() {
        super("Вычисление затрат");
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout(8, 8));

        // Данные подарков по компаниям (BYN)
        giftsByGreeter.put("Коммунарка", List.of(
                new GiftOption("Подарочный набор шоколада", 120),
                new GiftOption("«Птичье молоко» (ассорти)", 90),
                new GiftOption("Набор пралине «Классический»", 140),
                new GiftOption("Шоколадные конфеты премиум", 160)
        ));
        giftsByGreeter.put("Семь пятниц", List.of(
                new GiftOption("Подарочный сертификат 100 BYN", 100),
                new GiftOption("Набор вина (2 бутылки)", 180),
                new GiftOption("Набор виски + бокалы", 350),
                new GiftOption("Гастронабор закусок к вину", 120)
        ));
        giftsByGreeter.put("Apple", List.of(
                new GiftOption("Apple Gift Card (250 BYN)", 250),
                new GiftOption("AirPods (2nd Gen)", 450),
                new GiftOption("Apple Watch SE", 950),
                new GiftOption("iPad (10th Gen)", 1800)
        ));
        giftsByGreeter.put("Интеграл", List.of(
                new GiftOption("Памятный набор микросхем", 200),
                new GiftOption("Настольный сувенир с логотипом", 150),
                new GiftOption("Электронный набор для пайки", 220),
                new GiftOption("Индикаторный модуль (сувенир)", 180)
        ));

        // Левая панель: Компании
        JPanel greetersPanel = new JPanel(new GridLayout(0, 1, 4, 4));
        greetersPanel.setBorder(new TitledBorder("Компания (можно несколько)"));
        for (String g : greeters) {
            JCheckBox cb = new JCheckBox(g);
            greeterChecks.put(g, cb);
            greetersPanel.add(cb);
            cb.addActionListener(e -> {
                updateGiftListModel();
                recalc();
            });
        }

        // Правая верхняя панель: Подарки
        JPanel giftsPanel = new JPanel(new BorderLayout(4, 4));
        giftsPanel.setBorder(new TitledBorder("Подарок (зависит от выбранных компаний)"));
        giftList.setModel(giftListModel);
        giftList.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        giftList.addListSelectionListener(e -> { if (!e.getValueIsAdjusting()) recalc(); });
        giftsPanel.add(new JScrollPane(giftList), BorderLayout.CENTER);

        // Правая нижняя панель: Опции
        JPanel optionsPanel = new JPanel(new GridLayout(0, 1, 4, 4));
        optionsPanel.setBorder(new TitledBorder("Опции"));
        ButtonGroup concertGroup = new ButtonGroup();
        concertGroup.add(concertYes);
        concertGroup.add(concertNo);
        JPanel concertBox = new JPanel(new GridLayout(0, 1));
        concertBox.setBorder(new TitledBorder("Концерт (" + concertFixedCost + " BYN)"));
        concertBox.add(concertYes);
        concertBox.add(concertNo);
        concertYes.addActionListener(e -> recalc());
        concertNo.addActionListener(e -> recalc());
        loyalCustomer.addActionListener(e -> recalc());
        optionsPanel.add(concertBox);
        optionsPanel.add(loyalCustomer);

        // Правый вертикальный сплит: Подарки | Опции
        JSplitPane rightSplit = new JSplitPane(JSplitPane.VERTICAL_SPLIT, giftsPanel, optionsPanel);
        rightSplit.setResizeWeight(0.7);

        // Главный горизонтальный сплит: Компании | (Правый сплит)
        JSplitPane mainSplit = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, greetersPanel, rightSplit);
        mainSplit.setResizeWeight(0.3);

        // Итоговая панель
        JPanel bottomPanel = new JPanel(new BorderLayout(6, 6));
        totalLabel.setFont(totalLabel.getFont().deriveFont(Font.BOLD, 18f));
        bottomPanel.add(totalLabel, BorderLayout.NORTH);
        orderText.setEditable(false);
        orderText.setLineWrap(true);
        orderText.setWrapStyleWord(true);
        bottomPanel.add(new JScrollPane(orderText), BorderLayout.CENTER);

        // Сборка в одном окне
        add(mainSplit, BorderLayout.CENTER);
        add(bottomPanel, BorderLayout.SOUTH);

        // Инициализация
        updateGiftListModel();
        recalc();

        setSize(950, 650);
        setLocationRelativeTo(null);
    }

    private void updateGiftListModel() {
        Set<String> selectedNames = new HashSet<>();
        for (GiftOption go : giftList.getSelectedValuesList()) selectedNames.add(go.name);

        Map<String, GiftOption> union = new LinkedHashMap<>();
        boolean anyChecked = false;
        for (var entry : greeterChecks.entrySet()) {
            if (entry.getValue().isSelected()) {
                anyChecked = true;
                for (GiftOption go : giftsByGreeter.getOrDefault(entry.getKey(), List.of())) {
                    union.putIfAbsent(go.name, go);
                }
            }
        }
        giftListModel.clear();
        if (!anyChecked) return;

        for (GiftOption go : union.values()) giftListModel.addElement(go);

        List<Integer> toSelectIdx = new ArrayList<>();
        for (int i = 0; i < giftListModel.size(); i++) {
            if (selectedNames.contains(giftListModel.get(i).name)) {
                toSelectIdx.add(i);
            }
        }
        int[] indices = toSelectIdx.stream().mapToInt(x -> x).toArray();
        giftList.setSelectedIndices(indices);
    }

    private void recalc() {
        int giftsSum = 0;
        List<GiftOption> selectedGifts = giftList.getSelectedValuesList();
        for (GiftOption go : selectedGifts) giftsSum += go.price;

        int concert = concertYes.isSelected() ? concertFixedCost : 0;

        int greetersCost = 0;
        List<String> chosenGreeters = new ArrayList<>();
        for (var e : greeterChecks.entrySet()) {
            if (e.getValue().isSelected()) {
                chosenGreeters.add(e.getKey());
                greetersCost += greeterCost(e.getKey());
            }
        }

        int subtotal = giftsSum + concert + greetersCost;
        int discount = loyalCustomer.isSelected() ? (int) Math.round(subtotal * 0.10) : 0;
        int total = subtotal - discount;

        totalLabel.setText("Итого: " + total + " BYN");

        StringBuilder sb = new StringBuilder();
        sb.append("Компании: ").append(chosenGreeters.isEmpty() ? "—" : String.join(", ", chosenGreeters))
                .append(" (").append(greetersCost).append(" BYN)").append("\n");
        sb.append("Подарки: ").append(selectedGifts.isEmpty() ? "—" :
                        String.join(", ", selectedGifts.stream().map(g -> g.name + " (" + g.price + " BYN)").toList()))
                .append("\n");
        sb.append("Концерт: ").append(concertYes.isSelected() ? "Да" : "Нет")
                .append(" (").append(concert).append(" BYN)").append("\n");
        sb.append("Скидка лояльности: ").append(discount).append(" BYN").append("\n");
        sb.append("Итого к оплате: ").append(total).append(" BYN");
        orderText.setText(sb.toString());
    }

    private int greeterCost(String greeter) {
        return switch (greeter) {
            case "Коммунарка" -> 300;
            case "Семь пятниц" -> 250;
            case "Apple" -> 1200;
            case "Интеграл" -> 500;
            default -> 0;
        };
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new AwardCostApp().setVisible(true));
    }
}
