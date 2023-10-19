***Input BMP file sould be named as: input{k}.bmp***

```
make run VAR={k}

/* eg. below line will output the results from input1.bmp */
make run VAR=1
```

```
/* If makefile cannot work, do below */
g++ hw1.cpp -o hw1
./hw1 {k}
```
