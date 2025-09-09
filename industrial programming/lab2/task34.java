import java.util.*;
//Даны две действительные квадратные матрицы порядка n.
//Получить новую матрицу умножением элементов каждой строки первой матрицы на
//наибольшее из значений элементов соответствующей строки второй матрицы.
public class task34 {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        System.out.print("Введите n (порядок квадратных матриц): ");
        int n = sc.nextInt();

        double[][] A = new double[n][n];
        double[][] B = new double[n][n];

        System.out.println("Введите матрицу A (" + n + "x" + n + ") построчно:");
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                A[i][j] = sc.nextDouble();

        System.out.println("Введите матрицу B (" + n + "x" + n + ") построчно:");
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                B[i][j] = sc.nextDouble();

        double[][] C = new double[n][n];

        for (int i = 0; i < n; i++) {
            double rowMax = B[i][0];
            for (int j = 1; j < n; j++) {
                if (B[i][j] > rowMax) rowMax = B[i][j];
            }
            for (int j = 0; j < n; j++) {
                C[i][j] = A[i][j] * rowMax;
            }
        }

        System.out.println("Результирующая матрица C = A ∘ scale(max(B по строкам)):");
        printMatrix(C);
    }

    private static void printMatrix(double[][] M) {
        for (double[] row : M) {
            for (double v : row) {
                System.out.printf("%10.4f", v);
            }
            System.out.println();
        }
    }
}
