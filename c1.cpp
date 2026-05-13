#include <iostream>

// 递归方式计算斐波那契数列
int fibonacciRecursive(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacciRecursive(n - 1) + fibonacciRecursive(n - 2);
}

// 迭代方式计算斐波那契数列
int fibonacciIterative(int n) {
    if (n <= 1) {
        return n;
    }
    int prev = 0, curr = 1;
    for (int i = 2; i <= n; ++i) {
        int temp = curr;
        curr = prev + curr;
        prev = temp;
    }
    return curr;
}

int main() {
    std::cout << "Hello, World!" << std::endl;
    
    int n = 10;
    
    std::cout << "\n斐波那契数列前 " << n << " 项（迭代方式）: ";
    for (int i = 0; i < n; ++i) {
        std::cout << fibonacciIterative(i) << " ";
    }
    std::cout << std::endl;
    
    std::cout << "斐波那契数列第 " << n << " 项（递归方式）: " << fibonacciRecursive(n) << std::endl;
    
    return 0;
}
