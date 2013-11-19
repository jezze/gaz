struct astruct
{

    char val1;
    int val2;

};

int test_astruct(int x)
{

    struct astruct a;

    a.val1 = 12;
    a.val2 = 0;

    return x + a.val1;

}

int main()
{

    return test_astruct(20);

}

