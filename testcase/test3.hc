struct TestStruc {
    int fib,
    int sum
}

int fib(int a) {
    if (a == 1) {
        return 1;
    } else if (a == 2) {
        return 1;
    } else {
        return fib(a - 1) + fib(a - 2);
    }
}

int sum(int n) {
    int s;
    s = 0;
    int [10] end;
    int idx;
    idx = 0;
    while (n > 0) {
        s = s + n;
        n--;
        if (idx < 10) {
            end[idx] = s;
            idx++;
        } else {
            break;
        }
    }
    idx--;
    while (idx >= 0) {
        printi(end[idx]);
        idx--;
    }
    return s;
}

void simpleLoop() {
    int i;
    i = 10;
    while (i >= 0) {
        printi(i);
        i--;
    }
}

int hello_main() {
    TestStruc struc;
    struc.sum = sum(15);
    struc.fib = fib(33);
    printi(struc.sum);
    printi(struc.fib);
}