#include "AST.h"
#include "Lexer.h"
#include "Parser.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace toy;
namespace cl = llvm::cl;

namespace {
    enum Action
    {
        None,
        DumpAST
    };
} // namespace

static cl::opt<std::string> inputFilename(cl::Positional,
                                          cl::desc("<input toy file>"),
                                          cl::init("-"),
                                          cl::value_desc("filename"));

static cl::opt<enum Action>
    emitAction("emit", cl::desc("Select the kind of output desired"),
    cl::values(clEnumValN(DumpAST, "ast", "output the AST dump")));

std::unique_ptr<toy::ModuleAST> parseInputFile(llvm::StringRef filename){
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
        llvm::MemoryBuffer::getFileOrSTDIN(filename);
        if(std::error_code ec = fileOrErr.getError()) {
            llvm::errs() << "Could not open input file: " << ec.message()
                         << "\n";
            return nullptr;
        }
        auto buffer = fileOrErr.get()->getBuffer();
        LexerBuffer lexer(buffer.begin(), buffer.end(), std::string(filename));
        Parser parser(lexer);
        return parser.parseModule();
}

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv, "toy-mlir compiler\n");
    auto moduleAST = parseInputFile(inputFilename);

    if(!moduleAST)
        return 1;

    switch (emitAction)
    {
    case Action::DumpAST:
        dump(*moduleAST);
        return 0;

    default:
        llvm::errs() << "No action specified (parsing only), use -emit=<action>\n";
    }
    return 0;
}
