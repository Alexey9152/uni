import java.util.*;
//Пусть m(А, i) означает номер столбца матрицы A, в котором находится последний в строке минимум i-й строки.
//Проверить, что для заданной матрицы А выполняются неравенства m(A,1) ≤ m(A,2) ≤ ... ≤ m(A,m).
public class task20 {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        System.out.print("Введите количество строк m: ");
        int m = sc.nextInt();
        System.out.print("Введите количество столбцов n: ");
        int n = sc.nextInt();

        int[][] A = new int[m][n];
        System.out.println("Введите элементы матрицы построчно:");
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                A[i][j] = sc.nextInt();
            }
        }

        int[] lastMinCols = new int[m];
        for (int i = 0; i < m; i++) {
            lastMinCols[i] = lastMinCol1Based(A[i]);
        }

        boolean nonDecreasing = isNonDecreasing(lastMinCols);

        System.out.println("m(A, i) по строкам (1-based): " + Arrays.toString(lastMinCols));
        System.out.println("Условие m(A,1) ≤ m(A,2) ≤ ... ≤ m(A,m): " +
                (nonDecreasing ? "выполняется" : "НЕ выполняется"));
    }

    private static int lastMinCol1Based(int[] row) {
        int minVal = row[0];
        int pos = 0; // 0-based
        for (int j = 1; j < row.length; j++) {
            if (row[j] < minVal) {
                minVal = row[j];
                pos = j;
            } else if (row[j] == minVal) {
                pos = j;
            }
        }
        return pos + 1;
    }

    // Проверка неубывания последовательности
    private static boolean isNonDecreasing(int[] a) {
        for (int i = 1; i < a.length; i++) {
            if (a[i] < a[i - 1]) return false;
        }
        return true;
    }
}
