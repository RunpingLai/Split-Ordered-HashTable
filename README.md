# Split-Ordered-HashTable

To run tests:

```
make
./test
```

To change data structure:

change TEST_FLAG at line 16 of test.c

TEST_FLAG 0 : testing split-ordered list hashtable  
TEST_FLAG 1 : testing lock-based hashtable  
TEST_FLAG 2 : testing resizable lock-based hashtable (issue: deadlocks, still fixing)  
