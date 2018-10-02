CoGeCs - Low level language for JIT


### Function support 

* `store and load builtin functions for stack operations`
* `pointer type`

#### Stack frame
~~~~~~~~~~~~~~~~~~~~~~~~none
+---------------------+
| Parameter n         |
+---------------------+
| Paremeter n-1       |
+---------------------+
| Parameter 1         |
+---------------------+
| Return value address|
+---------------------+
| Return address      |
+---------------------+
~~~~~~~~~~~~~~~~~~~~~~~~