#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern FILE *yyin;
extern Block* programBlock;

int main(int argc, char **argv)
{
    FILE *inpFile = fopen(argv[1], "r");
    if (!inpFile)
    {
        cout << "Error opening File" << endl;
        return -1;
    }

    yyin = inpFile;

    yyparse();
    std::cout << programBlock << endl;
    // see http://comments.gmane.org/gmane.comp.compilers.llvm.devel/33877
    llvm::InitializeNativeTarget();
    CodeGenContext context;
    context.generateCode(*programBlock);
    context.runCode();
    
    system("pause");
    return 0;
}

