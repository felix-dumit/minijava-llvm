#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"

using namespace llvm;

namespace {
  struct DeadLoad : public FunctionPass {
    static char ID;
    DeadLoad() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      errs() << ">>>>>>>>>>>Function: " << F.getName() << '\n';

      bool changed = false;
      Instruction* lastInst = NULL;

      // loop through instructions of function
      for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i) {     	

      	Instruction* inst = &*i;

      	// if current inst is load
      	if(LoadInst* load =  dyn_cast<LoadInst>(inst)){
      		
      		// and previous inst is store
      		if(StoreInst* store =  dyn_cast<StoreInst>(lastInst)){

      			Value *valueStored = store->getOperand(0);
      			Value *addrStore = store->getOperand(1);
      			
      			// if they use the same address
      			if(addrStore == load->getOperand(0)){
      				
      				// go back to previous(store) instruction to compare with
      				// new next after deletion
      				--i;
      				// delete from parent and replace uses
      				load->eraseFromParent();
      				load->replaceAllUsesWith(valueStored);
      				changed = true;
      			}
      		}
      	}
      	
      	// update last instruction
      	lastInst = &*i;
     }      	

      return changed;
    }
  };
}

char DeadLoad::ID = 0;
static RegisterPass<DeadLoad> X("deadLoad", "DeadLoad Pass", false, false);
