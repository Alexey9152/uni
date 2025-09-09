import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
//Выведите номера столбцов, элементы каждого из которых образуют монотонную последовательность (монотонно убывающую или монотонно возрастающую).
public class task6 {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        System.out.print("Введите количество строк: ");
        int rows = sc.nextInt();
        System.out.print("Введите количество столбцов: ");
        int cols = sc.nextInt();

        int[][] a = new int[rows][cols];
        System.out.println("Введите элементы матрицы построчно:");
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                a[i][j] = sc.nextInt();
            }
        }

        // true — строгая монотонность (< или >), false — нестрогая (<= или >=)
        boolean strict = false;

        List<Integer> monotonicCols = new ArrayList<>();
        for (int j = 0; j < cols; j++) {
            if (rows <= 1 || isMonotonicColumn(a, rows, j, strict)) {
                monotonicCols.add(j + 1);
            }
        }

        if (monotonicCols.isEmpty()) {
            System.out.println("Монотонных столбцов нет.");
        } else {
            System.out.println("Номера монотонных столбцов: " + monotonicCols);
        }
    }

    private static boolean isMonotonicColumn(int[][] a, int rows, int j, boolean strict) {
        int dir = 0; // +1 — возр., -1 — убыв., 0 — не определено (все равны пока)
        for (int i = 1; i < rows; i++) {
            int prev = a[i - 1][j];
            int cur  = a[i][j];
            if (cur == prev) {
                if (strict) return false;
                continue;
            }
            int cmp = Integer.compare(cur, prev);
            if (dir == 0) {
                dir = cmp;
            } else if (cmp != dir && !(!strict && cmp == 0 )) {
                return false;
            }
        }
        return dir != 0 || !strict;
    }
}
