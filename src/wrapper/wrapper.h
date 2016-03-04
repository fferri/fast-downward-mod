#ifndef FASTDOWNWARD_WRAPPER_INCLUDED_H
#define FASTDOWNWARD_WRAPPER_INCLUDED_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>

class Term
{
public:
    Term(std::string functor);
    Term(std::string functor, std::vector<std::string> args);
    std::string functor() const;
    std::string arg(int i) const;
    int arity() const;
    std::string str() const;
    bool operator ==(const Term &o) const;
    bool operator <(const Term &o) const;

protected:
    std::string functor_;
    std::vector<std::string> args_;
};

struct StateDelta
{
    std::set<Term> addList;
    std::set<Term> delList;
};

class FastDownwardWrapper
{
public:
    FastDownwardWrapper(std::string fastdownward_dir, std::string fastdownward_script = "fast-downward.py", std::string python_executable = "/usr/bin/python");
    bool plan(std::string problem_file, int timeout, int kill_signal = SIGKILL);
    std::set<Term> parseState(std::string line);
    Term parseAction(std::string line);

    // interface functions:
    void reset();
    std::string getPlannerProperty(const std::string &s);
    bool setPlannerProperty(const std::string &var, const std::string &value);
    void clearDomain();
    void clearDomainSymbols();
    void clearDomainActions();
    void clearDomainProblems();
    void clearDomainInitialState();
    bool loadDomain(const std::string &filename);
    bool loadDomainSymbols(const std::string &file);
    bool loadDomainActions(const std::string &file);
    bool loadDomainProblems(const std::string &file);
    bool loadDomainProblemGoal(const std::string &file, const std::string &str);
    bool loadDomainProblemUpdateRules(const std::string &file, const std::string &str);
    bool loadDomainProblemMaintenanceRules(const std::string &file, const std::string &str);
    bool loadDomainProblemInitialState(const std::string &file, const std::string &str);
    bool loadDomainProblemInitialStateFacts(const std::string &file, const std::string &str);
    bool loadDomainInitialState(const std::string &file);
    bool loadDomainInitialStateFacts(const std::string &file);
    bool defineDomain(const std::string &str);
    bool defineDomainSymbols(const std::string &str);
    bool defineDomainActions(const std::string &str);
    bool defineDomainProblems(const std::string &str);
    bool defineDomainProblemGoal(const std::string &s, const std::string &t);
    bool defineDomainProblemUpdateRules(const std::string &s, const std::string &t);
    bool defineDomainProblemMaintenanceRules(const std::string &s, const std::string &t);
    bool defineDomainProblemInitialState(const std::string &s, const std::string &t);
    bool defineDomainProblemInitialStateFacts(const std::string &s, const std::string &t);
    bool defineDomainProblemInitialStateFactsFromList(const std::string &s, const std::vector<Term> &t);
    bool defineDomainInitialState(const std::string &str);
    bool defineDomainInitialStateFacts(const std::string &str);
    bool defineDomainInitialStateFactsFromList(const std::set<Term> &s);
    void clearPlan();
    void resetPlan();
    bool buildPlan();
    std::vector<Term> getPlan();
    std::set<Term> getPlanState();
    Term getPlanAction();
    bool isPlanDefined();
    bool isEndOfPlan();
    void advancePlan();
    bool setPlanProblem(const std::string &s);
    bool checkPlanActionPreconditions(const std::set<Term> &s);
    StateDelta getPlanActionEffects(const std::set<Term> &s);
    std::set<Term> getPlanActionEffectsState(const std::set<Term> &s);
    std::set<Term> getPlanInitialState();
    std::set<Term> getDomainInitialState();
    std::set<Term> getDomainProblemInitialState();
    StateDelta getStateDifference(const std::set<Term> &s1, const std::set<Term> &s2);

protected:
    std::string python_executable_;
    std::string fastdownward_dir_;
    std::string fastdownward_script_;

    std::stringstream domain_;
    std::stringstream problem_;

    std::vector<Term> plan_;
    bool plan_valid_;
    std::vector<std::set<Term> > plan_states_;
    int plan_pointer_; // current action in the plan
};

#endif // FASTDOWNWARD_WRAPPER_INCLUDED_H
