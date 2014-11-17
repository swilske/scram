/// @file risk_analysis.h
/// Contains the main system for performing analysis.
#ifndef SCRAM_SRC_RISK_ANALYISIS_H_
#define SCRAM_SRC_RISK_ANALYISIS_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <libxml++/libxml++.h>

#include "event.h"
#include "expression.h"
#include "fault_tree_analysis.h"
#include "probability_analysis.h"
#include "settings.h"
#include "uncertainty_analysis.h"

class RiskAnalysisTest;
class PerformanceTest;

namespace scram {

class Element;
class FaultTree;

/// @class RiskAnalysis
/// Main system that performs analyses.
class RiskAnalysis {
  friend class ::RiskAnalysisTest;
  friend class ::PerformanceTest;

 public:
  /// This constructor with configurations with the analysis.
  /// @param[in] config_file XML file with configurations.
  /// @todo Should be able to accept configurations from XML files.
  explicit RiskAnalysis(std::string config_file = "guess_yourself");

  /// Add set of settings for analysis.
  /// @param[in] settings Configured settings for analysis.
  void AddSettings(const Settings& settings) { settings_ = settings; }

  /// Reads one input file with the structure of analysis entities.
  /// Initializes the analysis from the given input file.
  /// Puts all events into their appropriate containers.
  /// @param[in] xml_file The formatted xml input file.
  /// @throws ValidationError if input contains errors.
  /// @throws ValueError if input values are not valid.
  /// @throws IOError if an input file is not accessable.
  /// @deprecated Use multiple file processing method instead.
  void ProcessInput(std::string xml_file);

  /// Reads input files with the structure of analysis entities.
  /// Initializes the analysis from the given input files.
  /// Puts all events into their appropriate containers.
  /// @param[in] xml_files The formatted xml input files.
  /// @throws ValidationError if input contains errors.
  /// @throws ValueError if input values are not valid.
  /// @throws IOError if an input file is not accessable.
  /// @todo May have default configurations for analysis off all input files.
  void ProcessInputFiles(const std::vector<std::string>& xml_files);

  /// Graphing or other visual resources for the analysis if applicable.
  /// Outputs instructions for graphviz dot to create a fault tree.
  /// Uses the standard output as destination.
  /// @note This function must be called only after initialization of the tree.
  inline void GraphingInstructions() { GraphingInstructions(std::cout); }

  /// Outputs graphing instructions to a specified file.
  /// param[out] output The output destination.
  /// @note This function must be called only after initialization of the tree.
  /// @throws IOError if the output file is not accessable.
  void GraphingInstructions(std::string output);

  /// Performs the main analysis operations.
  /// Analyzes the fault tree and performs computations.
  /// This function must be called only after initilizing the tree with or
  /// without its probabilities.
  /// @note Cut set generator: O_avg(N) O_max(N)
  void Analyze();

  /// Reports the results of analyses to the standard output.
  /// @note This function must be called only after Analyze() function.
  inline void Report() { Report(std::cout); }

  /// Reports the results of analysis to a specified output destination.
  /// @note This function must be called only after Analyze() function.
  /// param[out] output The output destination.
  /// @throws IOError if the output file is not accessable.
  void Report(std::string output);

 private:
  typedef boost::shared_ptr<Element> ElementPtr;
  typedef boost::shared_ptr<Event> EventPtr;
  typedef boost::shared_ptr<Gate> GatePtr;
  typedef boost::shared_ptr<PrimaryEvent> PrimaryEventPtr;
  typedef boost::shared_ptr<BasicEvent> BasicEventPtr;
  typedef boost::shared_ptr<HouseEvent> HouseEventPtr;
  typedef boost::shared_ptr<FaultTree> FaultTreePtr;
  typedef boost::shared_ptr<Expression> ExpressionPtr;
  typedef boost::shared_ptr<Parameter> ParameterPtr;
  typedef boost::shared_ptr<FaultTreeAnalysis> FaultTreeAnalysisPtr;
  typedef boost::shared_ptr<ProbabilityAnalysis> ProbabilityAnalysisPtr;
  typedef boost::shared_ptr<UncertaintyAnalysis> UncertaintyAnalysisPtr;

  /// Reads one input file with the structure of analysis entities.
  /// Initializes the analysis from the given input file.
  /// Puts all events into their appropriate containers.
  /// @param[in] xml_file The formatted xml input file.
  /// @throws ValidationError if input contains errors.
  /// @throws ValueError if input values are not valid.
  /// @throws IOError if an input file is not accessable.
  /// @todo May have default configurations for analysis off all input files.
  void ProcessInputFile(std::string xml_file);

  /// Attaches attributes and label to the elements of the analysis.
  /// These attributes are not XML attributes but OpenPSA format defined
  /// arbitrary attributes and label that can be attached to many analysis
  /// elements.
  /// @param[in] element_node XML element.
  /// @param[out] element The object that needs attributes and label.
  void AttachLabelAndAttributes(const xmlpp::Element* element_node,
                                const ElementPtr& element);

  /// Defines and adds a gate for this analysis.
  /// @param[in] gate_node XML element defining the gate.
  /// @param[out] ft FaultTree under which this gate is defined.
  void DefineGate(const xmlpp::Element* gate_node, const FaultTreePtr& ft);

  /// Processes the formula of a gate to be defined.
  /// Currently only one layer formula is supported.
  /// @param[in] gate The main gate to be defined with the formula.
  /// @param[in] events The xml node list of children of the gate definition.
  void ProcessFormula(const GatePtr& gate, const xmlpp::NodeSet& events);

  /// Process [event name=id] cases inside of a one layer gate description.
  /// @param[in] gate The main gate to be defined with the event as its child.
  /// @param[out] child The child the currently processed gate.
  /// @returns true if the child type is identified and it is update.
  /// @returns false if child type cannot be identified and it is saved for
  ///                late definition.
  bool ProcessFormulaEvent(const GatePtr& gate, EventPtr& child);

  /// Process [basic-event name=id] cases inside of a one layer
  /// gate description.
  /// @param[in] event XML element defining this event.
  /// @param[in] gate The main gate to be defined with the event as its child.
  /// @param[out] child The child the currently processed gate.
  void ProcessFormulaBasicEvent(const xmlpp::Element* event,
                                const GatePtr& gate, EventPtr& child);

  /// Process [house-event name=id] cases inside of a one layer
  /// gate description.
  /// @param[in] event XML element defining this event.
  /// @param[in] gate The main gate to be defined with the event as its child.
  /// @param[out] child The child the currently processed gate.
  void ProcessFormulaHouseEvent(const xmlpp::Element* event,
                                const GatePtr& gate, EventPtr& child);

  /// Process [gate name=id]cases inside of a one layer
  /// gate description.
  /// @param[in] event XML element defining this event.
  /// @param[in] gate The main gate to be defined with the event as its child.
  /// @param[out] child The child the currently processed gate.
  void ProcessFormulaGate(const xmlpp::Element* event, const GatePtr& gate,
                          EventPtr& child);

  /// Defines and adds a basic event for this analysis.
  /// @param[in] event_node XML element defining the event.
  void DefineBasicEvent(const xmlpp::Element* event_node);

  /// Defines and adds a house event for this analysis.
  /// @param[in] event_node XML element defining the event.
  void DefineHouseEvent(const xmlpp::Element* event_node);

  /// Defines a variable or parameter.
  /// @param[in] param_node XML element defining the parameter.
  void DefineParameter(const xmlpp::Element* param_node);

  /// Processes Expression definitions in input file.
  /// @param[in] expr_element XML expression element containing the definition.
  /// @param[out] expression Expression described in XML input expression node.
  /// @throws ValidationError if the expression is not supported.
  void GetExpression(const xmlpp::Element* expr_element,
                     ExpressionPtr& expression);

  /// Processes Constant Expression definitions in input file.
  /// @param[in] expr_element XML expression element containing the definition.
  /// @param[out] expression Expression described in XML input expression node.
  /// @returns true if expression was found and processed.
  bool GetConstantExpression(const xmlpp::Element* expr_element,
                             ExpressionPtr& expression);

  /// Processes Prameter Expression definitions in input file.
  /// @param[in] expr_element XML expression element containing the definition.
  /// @param[out] expression Expression described in XML input expression node.
  /// @returns true if expression was found and processed.
  bool GetParameterExpression(const xmlpp::Element* expr_element,
                              ExpressionPtr& expression);

  /// Processes Distribution deviate expression definitions in input file.
  /// @param[in] expr_element XML expression element containing the definition.
  /// @param[out] expression Expression described in XML input expression node.
  /// @returns true if expression was found and processed.
  bool GetDeviateExpression(const xmlpp::Element* expr_element,
                            ExpressionPtr& expression);

  /// Manages events that are defined late. That is, the id appears as
  /// [event name="id"] before any of definition inside a formula.
  /// The late definition should update if this event is a gate or primary
  /// event. In addition, all the parents of this late defined event are
  /// notified to include one child.
  /// @param[in] event Event that is defined late.
  /// @returns true if the given event is indeed a late one and
  ///               database update operations are performed accordingly.
  /// @returns false if the given event is not late and no action was taken.
  bool UpdateIfLateEvent(const EventPtr& event);

  /// Defines a fault tree for the analysis.
  /// @param[in] ft_node XML element defining the fault tree.
  void DefineFaultTree(const xmlpp::Element* ft_node);

  /// Processes model data with definitions of events and analysis.
  /// @param[in] model_data XML node with model data description.
  void ProcessModelData(const xmlpp::Element* model_data);

  /// Validates if the initialization of the analysis is successful.
  /// This validation process also generates optional warnings.
  /// @throws ValidationError if the initialization contains mistakes.
  void ValidateInitialization();

  /// Checks for problems with gates, events, expressions, and parameters.
  /// @throws ValidationError if the first layer members contain mistakes.
  void CheckFirstLayer();

  /// Checks for problems with analysis containers, such as fault trees,
  /// event trees, common cause groups, and others that use the first layer
  /// members.
  /// @throws ValidationError if the second layer members contain mistakes.
  void CheckSecondLayer();

  /// Verifies if gates are initialized correctly.
  /// @returns A warning message with a list of all bad gates with problems.
  /// @returns An empty string for no problems detected.
  std::string CheckAllGates();

  /// Checks if a gate is initialized correctly.
  /// @returns A warning message with the problem description.
  /// @returns An empty string for no problems detected.
  std::string CheckGate(const GatePtr& event);

  /// Checks if an Inhibit gate is initialized correctly.
  /// @returns A warning message with the problem description.
  /// @returns An empty string for no problems detected.
  std::string CheckInhibitGate(const GatePtr& event);

  /// @returns Formatted error message with house, basic, or other events
  /// that are not defined.
  /// @returns An empty string for no problems detected.
  std::string CheckMissingEvents();

  /// @returns Formatted error message with missing parameter names.
  /// @returns An empty string for no problems detected.
  std::string CheckMissingParameters();

  /// Validates expressions and anything that is dependent on them, such
  /// as paramters and basic events.
  /// @throws ValidationError if any problems detected with expressions.
  void ValidateExpressions();

  /// Provides graphing instructions for each fault tree initialized in
  /// the analysis.
  /// @param[out] out The output stream.
  void GraphingInstructions(std::ostream& out);

  /// Reports all the results generated by all the analyses.
  /// @param[out] out The output stream.
  void Report(std::ostream& out);

  /// Container of original names of to be determined events
  /// with capitalizations.
  std::map<std::string, std::string> tbd_orig_ids_;

  /// List of all valid gates.
  std::set<std::string> gate_types_;

  /// List of all valid types of primary events.
  std::set<std::string> types_;

  /// Container for fully defined gates.
  boost::unordered_map<std::string, GatePtr> gates_;

  /// Container for fully defined primary events.
  boost::unordered_map<std::string, PrimaryEventPtr> primary_events_;

  /// Container for fully defined basic events.
  boost::unordered_map<std::string, BasicEventPtr> basic_events_;

  /// Events to be defined with their parents saved for later.
  boost::unordered_map<std::string, std::vector<GatePtr> > tbd_events_;

  /// Gates to be defined.
  boost::unordered_map<std::string, GatePtr> tbd_gates_;

  /// Basic events to be defined.
  boost::unordered_map<std::string, BasicEventPtr> tbd_basic_events_;

  /// House events to be defined.
  boost::unordered_map<std::string, HouseEventPtr> tbd_house_events_;

  /// Container for excess primary events not in the analysis.
  /// This container is for warning in case the input is formed not as intended.
  std::set<PrimaryEventPtr> orphan_primary_events_;

  /// Container for defined parameters or variables.
  boost::unordered_map<std::string, ParameterPtr> parameters_;

  /// Container for to-be-defined parameters or variables.
  boost::unordered_map<std::string, ParameterPtr> tbd_parameters_;

  /// Map of valid units for parameters.
  std::map<std::string, Units> units_;

  /// Container for defined expressions.
  std::set<ExpressionPtr> expressions_;

  /// A collection of fault trees for analysis.
  std::map<std::string, FaultTreePtr> fault_trees_;

  /// Fault tree analyses that are performed.
  std::vector<FaultTreeAnalysisPtr> ftas_;

  /// Probability analyses that are performed.
  std::vector<ProbabilityAnalysisPtr> prob_analyses_;

  /// Uncertainty analyses that are performed.
  std::vector<UncertaintyAnalysisPtr> uncertainty_analyses_;

  /// Indicator if probability calculations are requested.
  bool prob_requested_;

  /// Settings for analysis.
  Settings settings_;

  /// Mission time expression.
  boost::shared_ptr<MissionTime> mission_time_;
};

}  // namespace scram

#endif  // SCRAM_SRC_RISK_ANALYSIS_H_
