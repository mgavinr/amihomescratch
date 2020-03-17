Some code examples
===

main
---
```
void main(argc, argv)
 int argc;
 union { char* args[]; struct WBStartup* msg; } argv;
 {
    if (argv == 0) { /// check argv.msg->sm_ArgLst[0]->wa_name; }
    else { /// check argv.args[0]; }
 }
 ```
 
 
