***Input BMP file sould be named as: input{k}.bmp***

Run 3 task at once
```
make run VAR={k}

/* eg. below line will output the results from input1.bmp */
make run VAR=1
```

If makefile cannot work, run below:
```
g++ hw1.cpp -o hw1
./hw1 {k}
```

Or you can run each task one by one:
```
g++ Resolution.cpp -o reso
./reso {k}

g++ FlipHokrizontally.cpp -o flip
./flip {k}

g++ Scaling.cpp -o scaling
./scaling {k}
```