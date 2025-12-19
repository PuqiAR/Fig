from time import time as tt

def fib(x:int) -> int:
    if x <= 1:  return x;
    return fib(x-1) + fib(x-2)

if __name__ == '__main__':
    t0 = tt()
    result = fib(25)
    t1 = tt()
    print('cost: ',t1-t0, 'result:', result)
