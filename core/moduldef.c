   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.24  06/05/06            */
   /*                                                     */
   /*                  DEFMODULE MODULE                   */
   /*******************************************************/

/*************************************************************/
/* Purpose: Defines basic defmodule primitive functions such */
/*   as allocating and deallocating, traversing, and finding */
/*   defmodule data structures.                              */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*            Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*************************************************************/

#define _MODULDEF_SOURCE_

#include "setup.h"

#include <stdio.h>
#include <string.h>
#define _STDIO_INCLUDED_

#include "memalloc.h"
#include "constant.h"
#include "router.h"
#include "extnfunc.h"
#include "argacces.h"
#include "constrct.h"
#include "modulpsr.h"
#include "modulcmp.h"
#include "modulbsc.h"
#include "utility.h"
#include "envrnmnt.h"

#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "bload.h"
#include "modulbin.h"
#endif

#include "moduldef.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

#if (! RUN_TIME)
   static void                       ReturnDefmodule(void *,struct defmodule *,intBool);
#endif
   static void                       DeallocateDefmoduleData(void *);

/**************************************************************/
/* InitializeDefmodules: Initializes the defmodule construct. */
/**************************************************************/
globle void AllocateDefmoduleGlobals(
  void *theEnv,
  EXEC_STATUS)
  {
   AllocateEnvironmentData(theEnv,execStatus,DEFMODULE_DATA,sizeof(struct defmoduleData),NULL);
   AddEnvironmentCleanupFunction(theEnv,execStatus,"defmodules",DeallocateDefmoduleData,-1000);
   DefmoduleData(theEnv)->CallModuleChangeFunctions = TRUE;
   DefmoduleData(theEnv)->MainModuleRedefinable = TRUE;
  }

/****************************************************/
/* DeallocateDefmoduleData: Deallocates environment */
/*    data for the defmodule construct.             */
/****************************************************/
static void DeallocateDefmoduleData(
  void *theEnv,
  EXEC_STATUS)
  {
   struct moduleStackItem *tmpMSPtr, *nextMSPtr;
   struct moduleItem *tmpMIPtr, *nextMIPtr;
#if (! RUN_TIME) && (! BLOAD_ONLY)
   struct defmodule *tmpDMPtr, *nextDMPtr;
   struct portConstructItem *tmpPCPtr, *nextPCPtr;
#endif
#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE) && (! RUN_TIME)
   int i;
   size_t space;
#endif

#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE) && (! RUN_TIME)
   for (i = 0; i < DefmoduleData(theEnv)->BNumberOfDefmodules; i++)
     {
      if (DefmoduleData(theEnv)->DefmoduleArray[i].itemsArray != NULL)
        { 
         rm(theEnv,execStatus,DefmoduleData(theEnv)->DefmoduleArray[i].itemsArray,
            sizeof(void *) * GetNumberOfModuleItems(theEnv));
        }
     }

   space = DefmoduleData(theEnv)->BNumberOfDefmodules * sizeof(struct defmodule);
   if (space != 0) 
     {
      genfree(theEnv,execStatus,(void *) DefmoduleData(theEnv)->DefmoduleArray,space);
      DefmoduleData(theEnv)->ListOfDefmodules = NULL;
     }

   space = DefmoduleData(theEnv)->NumberOfPortItems * sizeof(struct portItem);
   if (space != 0) genfree(theEnv,execStatus,(void *) DefmoduleData(theEnv)->PortItemArray,space);
#endif

#if (! RUN_TIME) && (! BLOAD_ONLY)
   tmpDMPtr = DefmoduleData(theEnv)->ListOfDefmodules;
   while (tmpDMPtr != NULL)
     {
      nextDMPtr = tmpDMPtr->next;
      ReturnDefmodule(theEnv,execStatus,tmpDMPtr,TRUE);
      tmpDMPtr = nextDMPtr;
     }

   tmpPCPtr = DefmoduleData(theEnv)->ListOfPortConstructItems;
   while (tmpPCPtr != NULL)
     {
      nextPCPtr = tmpPCPtr->next;
      rtn_struct(theEnv,execStatus,portConstructItem,tmpPCPtr);
      tmpPCPtr = nextPCPtr;
     }
#endif

   tmpMSPtr = DefmoduleData(theEnv)->ModuleStack;
   while (tmpMSPtr != NULL)
     {
      nextMSPtr = tmpMSPtr->next;
      rtn_struct(theEnv,execStatus,moduleStackItem,tmpMSPtr);
      tmpMSPtr = nextMSPtr;
     }

   tmpMIPtr = DefmoduleData(theEnv)->ListOfModuleItems;
   while (tmpMIPtr != NULL)
     {
      nextMIPtr = tmpMIPtr->next;
      rtn_struct(theEnv,execStatus,moduleItem,tmpMIPtr);
      tmpMIPtr = nextMIPtr;
     }
     
#if (! RUN_TIME) && (! BLOAD_ONLY)
   DeallocateCallList(theEnv,execStatus,DefmoduleData(theEnv)->AfterModuleDefinedFunctions);
#endif
   DeallocateCallList(theEnv,execStatus,DefmoduleData(theEnv)->AfterModuleChangeFunctions);
  }
  
/**************************************************************/
/* InitializeDefmodules: Initializes the defmodule construct. */
/**************************************************************/
globle void InitializeDefmodules(
  void *theEnv,
  EXEC_STATUS)
  {
   DefmoduleBasicCommands(theEnv);

#if (! RUN_TIME)
   CreateMainModule(theEnv);
#endif

#if DEFMODULE_CONSTRUCT && (! RUN_TIME) && (! BLOAD_ONLY)
   AddConstruct(theEnv,execStatus,"defmodule","defmodules",ParseDefmodule,NULL,NULL,NULL,NULL,
                                                        NULL,NULL,NULL,NULL,NULL);
#endif

#if (! RUN_TIME) && DEFMODULE_CONSTRUCT
   EnvDefineFunction2(theEnv,execStatus,"get-current-module", 'w',
                   PTIEF GetCurrentModuleCommand,
                   "GetCurrentModuleCommand", "00");

   EnvDefineFunction2(theEnv,execStatus,"set-current-module", 'w',
                   PTIEF SetCurrentModuleCommand,
                   "SetCurrentModuleCommand", "11w");
#endif
  }

/******************************************************/
/* RegisterModuleItem: Called to register a construct */
/*   which can be placed within a module.             */
/******************************************************/
globle int RegisterModuleItem(
   void *theEnv,
  EXEC_STATUS,
   char *theItem,
   void *(*allocateFunction)(void *),
   void (*freeFunction)(void *,void *),
   void *(*bloadModuleReference)(void *,int),
   void  (*constructsToCModuleReference)(void *,FILE *,int,int,int),
   void *(*findFunction)(void *,char *))
  {
   struct moduleItem *newModuleItem;

   newModuleItem = get_struct(theEnv,execStatus,moduleItem);
   newModuleItem->name = theItem;
   newModuleItem->allocateFunction = allocateFunction;
   newModuleItem->freeFunction = freeFunction;
   newModuleItem->bloadModuleReference = bloadModuleReference;
   newModuleItem->constructsToCModuleReference = constructsToCModuleReference;
   newModuleItem->findFunction = findFunction;
   newModuleItem->moduleIndex = DefmoduleData(theEnv)->NumberOfModuleItems++;
   newModuleItem->next = NULL;

   if (DefmoduleData(theEnv)->LastModuleItem == NULL)
     {
      DefmoduleData(theEnv)->ListOfModuleItems = newModuleItem;
      DefmoduleData(theEnv)->LastModuleItem = newModuleItem;
     }
   else
     {
      DefmoduleData(theEnv)->LastModuleItem->next = newModuleItem;
      DefmoduleData(theEnv)->LastModuleItem = newModuleItem;
     }

   return(newModuleItem->moduleIndex);
  }

/***********************************************************/
/* GetListOfModuleItems: Returns the list of module items. */
/***********************************************************/
globle struct moduleItem *GetListOfModuleItems(
  void *theEnv,
  EXEC_STATUS)
  {
   return (DefmoduleData(theEnv)->ListOfModuleItems);
  }

/***************************************************************/
/* GetNumberOfModuleItems: Returns the number of module items. */
/***************************************************************/
globle int GetNumberOfModuleItems(
  void *theEnv,
  EXEC_STATUS)
  {
   return (DefmoduleData(theEnv)->NumberOfModuleItems);
  }

/********************************************************/
/* FindModuleItem: Finds the module item data structure */
/*   corresponding to the specified name.               */
/********************************************************/
globle struct moduleItem *FindModuleItem(
  void *theEnv,
  EXEC_STATUS,
  char *theName)
  {
   struct moduleItem *theModuleItem;

   for (theModuleItem = DefmoduleData(theEnv)->ListOfModuleItems;
        theModuleItem != NULL;
        theModuleItem = theModuleItem->next)
     { if (strcmp(theModuleItem->name,theName) == 0) return(theModuleItem); }

   return(NULL);
  }

/******************************************/
/* EnvGetCurrentModule: Returns a pointer */
/*   to the current module.               */
/******************************************/
globle void *EnvGetCurrentModule(
  void *theEnv,
  EXEC_STATUS)
  {
   return ((void *) DefmoduleData(theEnv)->CurrentModule);
  }

/**************************************************************/
/* EnvSetCurrentModule: Sets the value of the current module. */
/**************************************************************/
globle void *EnvSetCurrentModule(
  void *theEnv,
  EXEC_STATUS,
  void *xNewValue)
  {
   struct defmodule *newValue = (struct defmodule *) xNewValue;
   struct callFunctionItem *changeFunctions;
   void *rv;

   /*=============================================*/
   /* Change the current module to the specified  */
   /* module and save the previous current module */
   /* for the return value.                       */
   /*=============================================*/

   rv = (void *) DefmoduleData(theEnv)->CurrentModule;
   DefmoduleData(theEnv)->CurrentModule = newValue;

   /*==========================================================*/
   /* Call the list of registered functions that need to know  */
   /* when the module has changed. The module change functions */
   /* should only be called if this is a "real" module change. */
   /* Many routines temporarily change the module to look for  */
   /* constructs, etc. The SaveCurrentModule function will     */
   /* disable the change functions from being called.          */
   /*==========================================================*/

   if (DefmoduleData(theEnv)->CallModuleChangeFunctions)
     {
      DefmoduleData(theEnv)->ModuleChangeIndex++;
      changeFunctions = DefmoduleData(theEnv)->AfterModuleChangeFunctions;
      while (changeFunctions != NULL)
        {
         (* (void (*)(void *)) changeFunctions->func)(theEnv);
         changeFunctions = changeFunctions->next;
        }
     }

   /*=====================================*/
   /* Return the previous current module. */
   /*=====================================*/

   return(rv);
  }

/********************************************************/
/* SaveCurrentModule: Saves current module on stack and */
/*   prevents SetCurrentModule() from calling change    */
/*   functions                                          */
/********************************************************/
globle void SaveCurrentModule(
  void *theEnv,
  EXEC_STATUS)
  {
   MODULE_STACK_ITEM *tmp;

   tmp = get_struct(theEnv,execStatus,moduleStackItem);
   tmp->changeFlag = DefmoduleData(theEnv)->CallModuleChangeFunctions;
   DefmoduleData(theEnv)->CallModuleChangeFunctions = FALSE;
   tmp->theModule = DefmoduleData(theEnv)->CurrentModule;
   tmp->next = DefmoduleData(theEnv)->ModuleStack;
   DefmoduleData(theEnv)->ModuleStack = tmp;
  }

/**********************************************************/
/* RestoreCurrentModule: Restores saved module and resets */
/*   ability of SetCurrentModule() to call changed        */
/*   functions to previous state                          */
/**********************************************************/
globle void RestoreCurrentModule(
  void *theEnv,
  EXEC_STATUS)
  {
   MODULE_STACK_ITEM *tmp;

   tmp = DefmoduleData(theEnv)->ModuleStack;
   DefmoduleData(theEnv)->ModuleStack = tmp->next;
   DefmoduleData(theEnv)->CallModuleChangeFunctions = tmp->changeFlag;
   DefmoduleData(theEnv)->CurrentModule = tmp->theModule;
   rtn_struct(theEnv,execStatus,moduleStackItem,tmp);
  }

/*************************************************************/
/* GetModuleItem: Returns the data pointer for the specified */
/*   module item in the specified module. If no module is    */
/*   indicated, then the module item for the current module  */
/*   is returned.                                            */
/*************************************************************/
globle void *GetModuleItem(
  void *theEnv,
  EXEC_STATUS,
  struct defmodule *theModule,
  int moduleItemIndex)
  {
   if (theModule == NULL)
     {
      if (DefmoduleData(theEnv)->CurrentModule == NULL) return(NULL);
      theModule = DefmoduleData(theEnv)->CurrentModule;
     }

   if (theModule->itemsArray == NULL) return (NULL);
   return ((void *) theModule->itemsArray[moduleItemIndex]);
  }

/************************************************************/
/* SetModuleItem: Sets the data pointer for the specified   */
/*   module item in the specified module. If no module is   */
/*   indicated, then the module item for the current module */
/*   is returned.                                           */
/************************************************************/
globle void SetModuleItem(
  void *theEnv,
  EXEC_STATUS,
  struct defmodule *theModule,
  int moduleItemIndex,
  void *newValue)
  {
   if (theModule == NULL)
     {
      if (DefmoduleData(theEnv)->CurrentModule == NULL) return;
      theModule = DefmoduleData(theEnv)->CurrentModule;
     }

   if (theModule->itemsArray == NULL) return;
   theModule->itemsArray[moduleItemIndex] = (struct defmoduleItemHeader *) newValue;
  }

/******************************************************/
/* CreateMainModule: Creates the default MAIN module. */
/******************************************************/
globle void CreateMainModule(
  void *theEnv,
  EXEC_STATUS)
  {
   struct defmodule *newDefmodule;
   struct moduleItem *theItem;
   int i;
   struct defmoduleItemHeader *theHeader;

   /*=======================================*/
   /* Allocate the defmodule data structure */
   /* and name it the MAIN module.          */
   /*=======================================*/

   newDefmodule = get_struct(theEnv,execStatus,defmodule);
   newDefmodule->name = (SYMBOL_HN *) EnvAddSymbol(theEnv,execStatus,"MAIN");
   IncrementSymbolCount(newDefmodule->name);
   newDefmodule->next = NULL;
   newDefmodule->ppForm = NULL;
   newDefmodule->importList = NULL;
   newDefmodule->exportList = NULL;
   newDefmodule->bsaveID = 0L;
   newDefmodule->usrData = NULL;

   /*==================================*/
   /* Initialize the array for storing */
   /* the module's construct lists.    */
   /*==================================*/

   if (DefmoduleData(theEnv)->NumberOfModuleItems == 0) newDefmodule->itemsArray = NULL;
   else
     {
      newDefmodule->itemsArray = (struct defmoduleItemHeader **)
                                 gm2(theEnv,execStatus,sizeof(void *) * DefmoduleData(theEnv)->NumberOfModuleItems);
      for (i = 0, theItem = DefmoduleData(theEnv)->ListOfModuleItems;
           (i < DefmoduleData(theEnv)->NumberOfModuleItems) && (theItem != NULL);
           i++, theItem = theItem->next)
        {
         if (theItem->allocateFunction == NULL)
           { newDefmodule->itemsArray[i] = NULL; }
         else
           {
            newDefmodule->itemsArray[i] = (struct defmoduleItemHeader *)
                                          (*theItem->allocateFunction)(theEnv);
            theHeader = (struct defmoduleItemHeader *) newDefmodule->itemsArray[i];
            theHeader->theModule = newDefmodule;
            theHeader->firstItem = NULL;
            theHeader->lastItem = NULL;
           }
        }
     }

   /*=======================================*/
   /* Add the module to the list of modules */
   /* and make it the current module.       */
   /*=======================================*/

#if (! BLOAD_ONLY) && (! RUN_TIME) && DEFMODULE_CONSTRUCT
   SetNumberOfDefmodules(theEnv,execStatus,1L);
#endif

   DefmoduleData(theEnv)->LastDefmodule = newDefmodule;
   DefmoduleData(theEnv)->ListOfDefmodules = newDefmodule;
   EnvSetCurrentModule(theEnv,execStatus,(void *) newDefmodule);
  }

/*********************************************************************/
/* SetListOfDefmodules: Sets the list of defmodules to the specified */
/*   value. Normally used when initializing a run-time module or     */
/*   when bloading a binary file to install the list of defmodules.  */
/*********************************************************************/
globle void SetListOfDefmodules(
  void *theEnv,
  EXEC_STATUS,
  void *defmodulePtr)
  {
   DefmoduleData(theEnv)->ListOfDefmodules = (struct defmodule *) defmodulePtr;

   DefmoduleData(theEnv)->LastDefmodule = DefmoduleData(theEnv)->ListOfDefmodules;
   if (DefmoduleData(theEnv)->LastDefmodule == NULL) return;
   while (DefmoduleData(theEnv)->LastDefmodule->next != NULL) DefmoduleData(theEnv)->LastDefmodule = DefmoduleData(theEnv)->LastDefmodule->next;
  }

/********************************************************************/
/* EnvGetNextDefmodule: If passed a NULL pointer, returns the first */
/*   defmodule in the ListOfDefmodules. Otherwise returns the next  */
/*   defmodule following the defmodule passed as an argument.       */
/********************************************************************/
globle void *EnvGetNextDefmodule(
  void *theEnv,
  EXEC_STATUS,
  void *defmodulePtr)
  {
   if (defmodulePtr == NULL)
     { return((void *) DefmoduleData(theEnv)->ListOfDefmodules); }
   else
     { return((void *) (((struct defmodule *) defmodulePtr)->next)); }
  }

/*****************************************/
/* EnvGetDefmoduleName: Returns the name */
/*   of the specified defmodule.         */
/*****************************************/
#if WIN_BTC
#pragma argsused
#endif
globle char *EnvGetDefmoduleName(
  void *theEnv,
  EXEC_STATUS,
  void *defmodulePtr)
  { 
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(theEnv)
#endif

   return(ValueToString(((struct defmodule *) defmodulePtr)->name)); 
  }

/***************************************************/
/* EnvGetDefmodulePPForm: Returns the pretty print */
/*   representation of the specified defmodule.    */
/***************************************************/
#if WIN_BTC
#pragma argsused
#endif
globle char *EnvGetDefmodulePPForm(
  void *theEnv,
  EXEC_STATUS,
  void *defmodulePtr)
  { 
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(theEnv)
#endif

   return(((struct defmodule *) defmodulePtr)->ppForm); 
  }

#if (! RUN_TIME)

/***********************************************/
/* RemoveAllDefmodules: Removes all defmodules */
/*   from the current environment.             */
/***********************************************/
globle void RemoveAllDefmodules(
  void *theEnv,
  EXEC_STATUS)
  {
   struct defmodule *nextDefmodule;

   while (DefmoduleData(theEnv)->ListOfDefmodules != NULL)
     {
      nextDefmodule = DefmoduleData(theEnv)->ListOfDefmodules->next;
      ReturnDefmodule(theEnv,execStatus,DefmoduleData(theEnv)->ListOfDefmodules,FALSE);
      DefmoduleData(theEnv)->ListOfDefmodules = nextDefmodule;
     }

   DefmoduleData(theEnv)->CurrentModule = NULL;
   DefmoduleData(theEnv)->LastDefmodule = NULL;
  }

/************************************************************/
/* ReturnDefmodule: Returns the data structures associated  */
/*   with a defmodule construct to the pool of free memory. */
/************************************************************/
static void ReturnDefmodule(
  void *theEnv,
  EXEC_STATUS,
  struct defmodule *theDefmodule,
  intBool environmentClear)
  {
   int i;
   struct moduleItem *theItem;
   struct portItem *theSpec, *nextSpec;

   /*=====================================================*/
   /* Set the current module to the module being deleted. */
   /*=====================================================*/

   if (theDefmodule == NULL) return;
   
   if (! environmentClear)
     { EnvSetCurrentModule(theEnv,execStatus,(void *) theDefmodule); }

   /*============================================*/
   /* Call the free functions for the constructs */
   /* belonging to this module.                  */
   /*============================================*/

   if (theDefmodule->itemsArray != NULL)
     {
      if (! environmentClear)
        {
         for (i = 0, theItem = DefmoduleData(theEnv)->ListOfModuleItems;
              (i < DefmoduleData(theEnv)->NumberOfModuleItems) && (theItem != NULL);
              i++, theItem = theItem->next)
           {
            if (theItem->freeFunction != NULL)
              { (*theItem->freeFunction)(theEnv,execStatus,theDefmodule->itemsArray[i]); }
           }
        }

      rm(theEnv,execStatus,theDefmodule->itemsArray,sizeof(void *) * DefmoduleData(theEnv)->NumberOfModuleItems);
    }

   /*======================================================*/
   /* Decrement the symbol count for the defmodule's name. */
   /*======================================================*/

   if (! environmentClear)
     { DecrementSymbolCount(theEnv,execStatus,theDefmodule->name); }

   /*====================================*/
   /* Free the items in the import list. */
   /*====================================*/

   theSpec = theDefmodule->importList;
   while (theSpec != NULL)
     {
      nextSpec = theSpec->next;
      if (! environmentClear)
        {
         if (theSpec->moduleName != NULL) DecrementSymbolCount(theEnv,execStatus,theSpec->moduleName);
         if (theSpec->constructType != NULL) DecrementSymbolCount(theEnv,execStatus,theSpec->constructType);
         if (theSpec->constructName != NULL) DecrementSymbolCount(theEnv,execStatus,theSpec->constructName);
        }
      rtn_struct(theEnv,execStatus,portItem,theSpec);
      theSpec = nextSpec;
     }

   /*====================================*/
   /* Free the items in the export list. */
   /*====================================*/

   theSpec = theDefmodule->exportList;
   while (theSpec != NULL)
     {
      nextSpec = theSpec->next;
      if (! environmentClear)
        {
         if (theSpec->moduleName != NULL) DecrementSymbolCount(theEnv,execStatus,theSpec->moduleName);
         if (theSpec->constructType != NULL) DecrementSymbolCount(theEnv,execStatus,theSpec->constructType);
         if (theSpec->constructName != NULL) DecrementSymbolCount(theEnv,execStatus,theSpec->constructName);
        }
      rtn_struct(theEnv,execStatus,portItem,theSpec);
      theSpec = nextSpec;
     }

   /*=========================================*/
   /* Free the defmodule pretty print string. */
   /*=========================================*/

   if (theDefmodule->ppForm != NULL)
     {
      rm(theEnv,execStatus,theDefmodule->ppForm,
         (int) sizeof(char) * (strlen(theDefmodule->ppForm) + 1));
     }
     
   /*=======================*/
   /* Return the user data. */
   /*=======================*/

   ClearUserDataList(theEnv,execStatus,theDefmodule->usrData);
   
   /*======================================*/
   /* Return the defmodule data structure. */
   /*======================================*/

   rtn_struct(theEnv,execStatus,defmodule,theDefmodule);
  }

#endif /* (! RUN_TIME) */

/**********************************************************************/
/* EnvFindDefmodule: Searches for a defmodule in the list of defmodules. */
/*   Returns a pointer to the defmodule if found, otherwise NULL.     */
/**********************************************************************/
globle void *EnvFindDefmodule(
  void *theEnv,
  EXEC_STATUS,
  char *defmoduleName)
  {
   struct defmodule *defmodulePtr;
   SYMBOL_HN *findValue;

   if ((findValue = (SYMBOL_HN *) FindSymbolHN(theEnv,execStatus,defmoduleName)) == NULL) return(NULL);

   defmodulePtr = DefmoduleData(theEnv)->ListOfDefmodules;
   while (defmodulePtr != NULL)
     {
      if (defmodulePtr->name == findValue)
        { return((void *) defmodulePtr); }

      defmodulePtr = defmodulePtr->next;
     }

   return(NULL);
  }

/*************************************************/
/* GetCurrentModuleCommand: H/L access routine   */
/*   for the get-current-module command.         */
/*************************************************/
globle void *GetCurrentModuleCommand(
  void *theEnv,
  EXEC_STATUS)
  {
   struct defmodule *theModule;

   EnvArgCountCheck(theEnv,execStatus,"get-current-module",EXACTLY,0);

   theModule = (struct defmodule *) EnvGetCurrentModule(theEnv);

   if (theModule == NULL) return((SYMBOL_HN *) EnvFalseSymbol(theEnv));

   return((SYMBOL_HN *) EnvAddSymbol(theEnv,execStatus,ValueToString(theModule->name)));
  }

/*************************************************/
/* SetCurrentModuleCommand: H/L access routine   */
/*   for the set-current-module command.         */
/*************************************************/
globle void *SetCurrentModuleCommand(
  void *theEnv,
  EXEC_STATUS)
  {
   DATA_OBJECT argPtr;
   char *argument;
   struct defmodule *theModule;
   SYMBOL_HN *defaultReturn;

   /*=====================================================*/
   /* Check for the correct number and type of arguments. */
   /*=====================================================*/

   theModule = ((struct defmodule *) EnvGetCurrentModule(theEnv));
   if (theModule == NULL) return((SYMBOL_HN *) EnvFalseSymbol(theEnv));

   defaultReturn = (SYMBOL_HN *) EnvAddSymbol(theEnv,execStatus,ValueToString(((struct defmodule *) EnvGetCurrentModule(theEnv))->name));

   if (EnvArgCountCheck(theEnv,execStatus,"set-current-module",EXACTLY,1) == -1)
     { return(defaultReturn); }

   if (EnvArgTypeCheck(theEnv,execStatus,"set-current-module",1,SYMBOL,&argPtr) == FALSE)
     { return(defaultReturn); }

   argument = DOToString(argPtr);

   /*================================================*/
   /* Set the current module to the specified value. */
   /*================================================*/

   theModule = (struct defmodule *) EnvFindDefmodule(theEnv,execStatus,argument);

   if (theModule == NULL)
     {
      CantFindItemErrorMessage(theEnv,execStatus,"defmodule",argument);
      return(defaultReturn);
     }

   EnvSetCurrentModule(theEnv,execStatus,(void *) theModule);

   /*================================*/
   /* Return the new current module. */
   /*================================*/

   return((SYMBOL_HN *) defaultReturn);
  }

/*************************************************/
/* AddAfterModuleChangeFunction: Adds a function */
/*   to the list of functions to be called after */
/*   a module change occurs.                     */
/*************************************************/
globle void AddAfterModuleChangeFunction(
  void *theEnv,
  EXEC_STATUS,
  char *name,
  void (*func)(void *),
  int priority)
  {
   DefmoduleData(theEnv)->AfterModuleChangeFunctions =
     AddFunctionToCallList(theEnv,execStatus,name,priority,func,DefmoduleData(theEnv)->AfterModuleChangeFunctions,TRUE);
  }

/************************************************/
/* IllegalModuleSpecifierMessage: Error message */
/*   for the illegal use of a module specifier. */
/************************************************/
globle void IllegalModuleSpecifierMessage(
  void *theEnv,
  EXEC_STATUS)
  {
   PrintErrorID(theEnv,execStatus,"MODULDEF",1,TRUE);
   EnvPrintRouter(theEnv,execStatus,WERROR,"Illegal use of the module specifier.\n");
  }


