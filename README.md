# MyCB

MyCB is an experimental C++ code browser based on Clang made as a side project
in order to understand the challenges of parsing C++ when it comes
to having proper software to do C++ code review.

## References

- https://clang.llvm.org/docs/RAVFrontendAction.html
- https://clang.llvm.org/docs/IntroductionToTheClangAST.html

Useful command:
```
clang -Xclang -ast-dump -fsyntax-only example.cc 
```
