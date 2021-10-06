# project 1
> In the first project, you are required to implement a SPL parser. Specifically, your parser will perform lexical analysis and syntax analysis on the SPL source code. You will use two powerful open-source tools, Flex1 and Bison2, to realize your parser. you will implement it by C programming language. In modern compiler design and implementation, lexical analysis and syntax analysis can be totally automated (so you don’t need to implement your NFA/DFA, or parsing algorithms), typical tools are Lex/Flex for generating lexical analyzer, and Yacc/Bison for generating parser. Though you can complete project 1 without too much theoretical knowledge, it doesn’t mean the theory is not important. You will learn how to recognize tokens using regular expressions and how to check code structures using context-free grammars in the lecture. As for your project, what you need to do is just specifying the regular expressions and grammar rules.
> One thing to note is that, you will finish the subsequent parts of your compiler on top of this work, thus it is important to keep your code maintainable and extensible.