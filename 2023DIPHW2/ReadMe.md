Run 3 task at once
```
make
make run
```

Or you can run each task one by one:
```
g++ Low-luminosity-enhancement.cpp 
./a.out 1 1
./a.out 1 2

g++ Denoise.cpp         
./a.out 3 1
./a.out 3 2
```