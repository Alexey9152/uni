import java.util.Scanner;

public class Main {
    public static double factorial(double n) {
        if (n <= 1) return 1;
        return n * factorial(n - 1);
    }
    public static void main(String[] args) {
        Scanner input = new Scanner(System.in);

        System.out.println("Введите х: ");
        double x = input.nextDouble();

        System.out.println("Введите k: ");
        int k = input.nextInt();

        double ans = 1, i = 1, n = 1;

        while (i >= Math.pow(10, -k)) {
            i = Math.pow(x, n) / factorial(n);
            n++;
            ans += i;
        }
        System.out.printf("Приближённое значение e^x: %.5f", ans);
    }
}