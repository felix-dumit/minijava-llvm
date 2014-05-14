#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Instructions.h"
#include <set>
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/CFG.h"

using namespace llvm;

namespace {

 DenseMap<const Instruction*, int> instMap;

  void print_elem(const Instruction* i) {
    errs() << instMap.lookup(i) << " ";
  }
    
  class genKill {
  public:
    std::set<const Instruction*> gen;
    std::set<const Instruction*> kill;
  };
  
  class beforeAfter {
  public:
    std::set<const Instruction*> before;
    std::set<const Instruction*> after;
  };


  struct Live : public FunctionPass {
    static char ID;
    Live() : FunctionPass(ID) {}

    void addToMap(Function &F) {
      static int id = 1;
      for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i, ++id){
        // Convert the iterator to a pointer, and insert the pair
        instMap.insert(std::make_pair(&*i, id));
        errs() << ">>>> " << id << ": " <<  i->getName() << "  \n";
	    }
    }

    void computeBBGenKill(Function &F, DenseMap<const BasicBlock*, genKill> &bbMap) 
    {
      for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
        genKill s;
        for (BasicBlock::iterator i = b->begin(), e = b->end(); i != e; ++i) {
          // The GEN set is the set of upwards-exposed uses:
          // pseudo-registers that are used in the block before being
          // defined. (Those will be the pseudo-registers that are defined
          // in other blocks, or are defined in the current block and used
          // in a phi function at the start of this block.) 
          unsigned n = i->getNumOperands();
          for (unsigned j = 0; j < n; j++) {
            Value *v = i->getOperand(j);
            if (isa<Instruction>(v)) {
              Instruction *op = cast<Instruction>(v);
              if (!s.kill.count(op))
                s.gen.insert(op);
            }
          }
          // For the KILL set, you can use the set of all instructions
          // that are in the block (which safely includes all of the
          // pseudo-registers assigned to in the block).
          s.kill.insert(&*i);
        }
        bbMap.insert(std::make_pair(&*b, s));
      }
    }

    // COMPUTA PARA CADA BLOCO O QUE TA VIVO ANTES E DEPOIS
    // Do this using a worklist algorithm (where the items in the worklist are basic blocks).
    void computeBBBeforeAfter(Function &F, DenseMap<const BasicBlock*, genKill> &bbGKMap,
                              DenseMap<const BasicBlock*, beforeAfter> &bbBAMap)
    {
      SmallVector<BasicBlock*, 32> workList;
      workList.push_back(--F.end());

      // REPETE ATE STACK VAZIA (NAO OUVER MUDANCA NO AFTER)
      while (!workList.empty()) {
        BasicBlock *b = workList.pop_back_val();
        beforeAfter b_beforeAfter = bbBAMap.lookup(b);
        // VE SE JA ADICIONOU O PREDECESSOR
        bool shouldAddPred = !bbBAMap.count(b);
        genKill b_genKill = bbGKMap.lookup(b);
        
        // Take the union of all successors
        //ACHA TODAS INSTRUCOES QUE ENTRAM NOS SUCESSORES DO BLOCK B
        std::set<const Instruction*> a;
        for (succ_iterator SI = succ_begin(b), E = succ_end(b); SI != E; ++SI) {
          std::set<const Instruction*> s(bbBAMap.lookup(*SI).before);
          a.insert(s.begin(), s.end());
        }

        // SE INSTRUCOES QUE SAEM DE B SAO DIFERENTES DAS QUE ENTRAM NOS SUCCS
        if (a != b_beforeAfter.after){
          shouldAddPred = true;
          b_beforeAfter.after = a;
          // before = after - KILL + GEN
          b_beforeAfter.before.clear();
          // AFTER - KILL
          std::set_difference(a.begin(), a.end(), b_genKill.kill.begin(), b_genKill.kill.end(),
                              std::inserter(b_beforeAfter.before, b_beforeAfter.before.end()));
          // + GEN
          b_beforeAfter.before.insert(b_genKill.gen.begin(), b_genKill.gen.end());
        }
        
        // SE B NAO TAVA NO MAP, OU NAO ESTAVA CERTO AINDA
        if (shouldAddPred)
          // ADICIONA TODOS OS PREDS DE B NA STACK
          for (pred_iterator PI = pred_begin(b), E = pred_end(b); PI != E; ++PI)
            workList.push_back(*PI);
      }
    }
    
    //COMPUTA PARA CADA INSTRUCAO O QUE TA VIVO ANTES E DEPOIS
    void computeIBeforeAfter(Function &F, DenseMap<const BasicBlock*, beforeAfter> &bbBAMap,
                              DenseMap<const Instruction*, beforeAfter> &iBAMap)
    {
      // PRA CADA BLOCK
      for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
        BasicBlock::iterator i = --b->end();
        std::set<const Instruction*> liveAfter(bbBAMap.lookup(b).after);
        std::set<const Instruction*> liveBefore(liveAfter);

        // PARA CADA INSTRUCAO DO BB
        while (true) {
          // before = after - KILL + GEN
          liveBefore.erase(i);

          // PARA OPERANDO DA INSTRUCAO
          unsigned n = i->getNumOperands();
          for (unsigned j = 0; j < n; j++) {
            Value *v = i->getOperand(j);
            if (isa<Instruction>(v))
              liveBefore.insert(cast<Instruction>(v));
          }

          beforeAfter ba;
          ba.before = liveBefore;
          ba.after = liveAfter;
          iBAMap.insert(std::make_pair(&*i, ba));

          // COMO VAI PRA INSTRUCAO ANTERIOR, O QUE TA VIVO ANTES EM I, EH O VIVO DEPOIS DE I-1
          liveAfter = liveBefore;
          if (i == b->begin())
            break;
          --i;
        }
      }
    }

    virtual bool runOnFunction(Function &F) {
       // Iterate over the instructions in F, creating a map from instruction address to unique integer.
      addToMap(F);

      bool changed = false;

      // LLVM Value classes already have use information. But for the sake of learning, we will implement the iterative algorithm.
      
      DenseMap<const BasicBlock*, genKill> bbGKMap;
      // For each basic block in the function, compute the block's GEN and KILL sets.
      computeBBGenKill(F, bbGKMap);

      DenseMap<const BasicBlock*, beforeAfter> bbBAMap;
      // For each basic block in the function, compute the block's liveBefore and liveAfter sets.
      computeBBBeforeAfter(F, bbGKMap, bbBAMap);

      DenseMap<const Instruction*, beforeAfter> iBAMap;
      computeIBeforeAfter(F, bbBAMap, iBAMap);

      errs() << "BEGIN:" << F.getName() << '\n';

      // LOOPA AS INSTRUCOES DE F
      for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i) {
        //PEGA O MAP COM AS INSTRUCOES VIVAS ANTES E DEPOIS PARA A INTRUCAO I
        beforeAfter s = iBAMap.lookup(&*i);
        errs() << "%" << instMap.lookup(&*i) << ": { ";
        std::for_each(s.before.begin(), s.before.end(), print_elem);
        errs() << "} { ";
        std::for_each(s.after.begin(), s.after.end(), print_elem);
        errs() << "}\n";
      }

      errs() << "END:" << F.getName() << '\n';

      return changed;
    }
  };
}

char Live::ID = 0;
static RegisterPass<Live> X("live", "Liveness Pass", false, false);
