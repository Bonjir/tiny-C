main()
{
    int Alice = 234, Bob, Charlie = 200;
    if(Alice > 100){
        Alice = Alice + 100 + Charlie;
    }else{
        Bob = Charlie + Charlie;
        show(Bob);
    }
    show(Alice);
}