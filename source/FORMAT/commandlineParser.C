// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/FORMAT/commandlineParser.h>

#ifdef BALL_OS_WINDOWS
	#include <winsock2.h>
#endif

using namespace BALL;
using namespace std;

const String BALL::CommandlineParser::NOT_FOUND = "parameter_not_found";
const list<String> BALL::CommandlineParser::EMTPY_LIST(0);

CommandlineParser::CommandlineParser(String tool_name, String tool_description, String tool_version, String build_date, String category)
{
	max_parname_length_ = 0;
	max_flagname_length_ = 0;
	tool_name_ = tool_name;
	tool_description_ = tool_description;
	tool_version_ = tool_version;
	build_date_ = build_date;
	tool_manual_ = "";
	tool_category_ = category;

	hostname_ = "";
	start_time_ = "";

	char hn[1024];
	hn[1023] = '\0';
	gethostname(hn, 1023);
	hostname_ = String(hn);

	time_t rawtime;
	tm* ptm;
	time(&rawtime);
	ptm = localtime(&rawtime);
	char buffer[150];
	strftime (buffer, 150, "%Y-%m-%d, %X (%Z)", ptm);
	start_time_ = String(buffer);

	// For the moment, register Galaxy's char-excapes here.
	// If necessary, we could create a function to allow switching to other char-excape schemes ...
	excaped_chars_.push_back(make_pair("__gt__",">"));
	excaped_chars_.push_back(make_pair("__lt__","<"));
	excaped_chars_.push_back(make_pair("__sq__","'"));
	excaped_chars_.push_back(make_pair("__dq__","\""));
	excaped_chars_.push_back(make_pair("__ob__","["));
	excaped_chars_.push_back(make_pair("__cb__","]"));
	excaped_chars_.push_back(make_pair("__oc__","{"));
	excaped_chars_.push_back(make_pair("__cc__","}"));
	excaped_chars_.push_back(make_pair("__at__","@"));
	excaped_chars_.push_back(make_pair("__cn__","\n"));
	excaped_chars_.push_back(make_pair("__cr__","\r"));
	excaped_chars_.push_back(make_pair("__tc__","\t"));
}


void CommandlineParser::registerParameter(String name, String description, ParameterType type, bool mandatory, String default_value)
{
	ParameterDescription pardes;
	pardes.name = name;
	pardes.description = description;
	pardes.mandatory = mandatory;
	pardes.type = type;
	registered_parameters_.insert(make_pair(name, pardes));
	original_parameter_order_.push_back(registered_parameters_.find(name));
	if (default_value != "")
	{
		list<String> alist;
		alist.push_back(default_value);
		parameter_map_[name] = alist;
	}
	Size size = name.size();
	if (type == INFILE)
	{
		size += 9;
	}
	else if (type == OUTFILE)
	{
		size += 10;
	}
	else if (type == BALL::INT)
	{
		size += 6;
	}
	else if (type == DOUBLE || type == STRING)
	{
		size += 9;
	}
	if (size > max_parname_length_) max_parname_length_ = size;
}


void CommandlineParser::registerFlag(String name, String description, bool default_gui_value)
{
	ParameterDescription pardes;
	pardes.name = name;
	pardes.description = description;
	pardes.mandatory = false;
	list<String> values;
	values.push_back("0");
	values.push_back("1");
	pardes.type = BALL::INT;
	registered_flags_.insert(make_pair(name, pardes));
	registered_flags_.find(name)->second.allowed_values = values;
	original_flag_order_.push_back(registered_flags_.find(name));
	if (name.size() > max_flagname_length_) max_flagname_length_ = name.size();
	if (default_gui_value)
	{
		list<String> alist;
		alist.push_back("1");
		parameter_map_[name] = alist;
	}
}


void CommandlineParser::registerAdvancedParameters(Options& options)
{
	if (options.getName().hasPrefix("ReferenceArea") ||
		options.getName().hasPrefix("PharmacophoreConstraint") )
	{
		return;
	}
	String category = options.getName();
	if (category != "" && category != "Docking-Settings")
	{
		for (Options::ConstIterator it = options.begin(); it != options.end(); it++)
		{
			const String& name = it->first;
			const ParameterDescription* pardes = options.getParameterDescription(name);
			if (!pardes)
			{
				continue;
			}

			registerParameter(name, pardes->description, pardes->type, false, it->second);
			registered_parameters_[name].advanced = true;
			registered_parameters_[name].allowed_values = pardes->allowed_values;
			registered_parameters_[name].category = category;
		}
	}

	for(StringHashMap<Options*>::Iterator it = options.beginSubcategories(); it != options.endSubcategories(); it++)
	{
		registerAdvancedParameters(*it->second);
	}
}


void CommandlineParser::setParameterRestrictions(String par_name, double min_value, double max_value)
{
	map<String, ParameterDescription>::iterator it = registered_parameters_.find(par_name);
	if (it == registered_parameters_.end())
	{
		throw BALL::Exception::GeneralException(__FILE__,__LINE__,"setParameterRestrictions error","You need to register a parameter before you can set restrictions for it!");
	}
	it->second.allowed_values.clear();
	it->second.allowed_values.push_back(String(min_value));
	it->second.allowed_values.push_back(String(max_value));
}

void CommandlineParser::setParameterRestrictions(String par_name, list<String>& allowed_values)
{
	map<String, ParameterDescription>::iterator it = registered_parameters_.find(par_name);
	if (it == registered_parameters_.end())
	{
		throw BALL::Exception::GeneralException(__FILE__,__LINE__,"setParameterRestrictions error","You need to register a parameter before you can set restrictions for it!");
	}
	it->second.allowed_values.clear();
	it->second.allowed_values = allowed_values;
}


void CommandlineParser::setSupportedFormats(String par_name, String supported_formats)
{
	vector<String> formats;
	supported_formats.split(formats, ",");
	map<String, ParameterDescription>::iterator it = registered_parameters_.find(par_name);
	if (it == registered_parameters_.end())
	{
		throw BALL::Exception::GeneralException(__FILE__,__LINE__,"setSupportedFormats error","You need to register a parameter before you can set supported formats for it!");
	}
	it->second.supported_formats.clear();
	for (Size i=0; i<formats.size(); i++)
	{
		it->second.supported_formats.push_back(formats[i].trim());
	}
}


void CommandlineParser::setOutputFormatSource(String output_parname, String input_parname)
{
	map<String, ParameterDescription>::iterator it = registered_parameters_.find(output_parname);
	if (it == registered_parameters_.end())
	{
		throw BALL::Exception::GeneralException(__FILE__,__LINE__,"setOutputFormatSource error","You need to register a parameter before you can set its format-source!");
	}
	it->second.output_format_source = input_parname;
}


void CommandlineParser::printToolInfo()
{
	cout<<endl;
	Size length = tool_name_.size()+tool_description_.size()+6;
	cout<<" "; for (Size i = 0; i < length; i++) cout<<"="; cout<<endl;
	cout<<"| "<<tool_name_<<" -- "<<tool_description_<<" |"<<endl;
	cout<<"|"; for (Size i = 0; i < length; i++) cout<<"="; cout<<"|"<<endl;
	cout<<"|"; for (Size i = 0; i < length; i++) cout<<" ";
	cout<<"|"<<endl;

	int n = length-tool_version_.size()-11;
	cout<<"| Version: "<<tool_version_;
	for (int i = 0; i < n; i++) cout<<" ";
	cout<<" |"<<endl;

	cout<<"| build date: "<<build_date_;
	n = length-build_date_.size()-14;
	for (int i = 0; i < n; i++) cout<<" ";
	cout<<" |"<<endl;

	n = length-hostname_.size()-18;
	cout<<"| execution host: "<<hostname_;
	for (int i = 0; i < n; i++) cout<<" ";
	cout<<" |"<<endl;

	n = length-start_time_.size()-18;
	cout<<"| execution time: "<<start_time_;
	for (int i = 0; i < n; i++) cout<<" ";
	cout<<" |"<<endl;

	cout<<" "; for (Size i = 0; i < length; i++) cout<<"-";
	cout<<endl<<endl;
}


void CommandlineParser::parse(int argc, char* argv[])
{
	registerFlag("help", "show help about parameters and flags of this program");
	registerParameter("write_par", "write xml parameter file for this tool", OUTFILE);
	registerParameter("par", "read parameters from parameter-xml-file", INFILE);

	printToolInfo();

	if (argc < 2)
	{
		printHelp();
		exit(1);
	}

	// Make a copy of the default-parameters
	map<String, list<String> > default_values = parameter_map_;
	parameter_map_.clear();

	bool name_read = 0;
	String current_par_name="";
	String token = "";
	start_command_ = "";
	for (int i = 0; i < argc; i++)
	{
		token = argv[i];
		token.trim();
		start_command_ += token;
		if (i < argc-1) start_command_ += " ";

		if (token[0] == '-' && !token.isFloat())
		{
			token.trimLeft("-");
			if (!name_read)
			{
				current_par_name = token;
				name_read = true;
			}
			else // command line flag
			{
				if (registered_flags_.find(current_par_name) != registered_flags_.end())
				{
					list<String> v;
					v.push_back("1");
					parameter_map_.insert(make_pair(current_par_name, v));
					current_par_name = token;
					name_read = true;
				}
				else
				{
					if (registered_parameters_.find(current_par_name) != registered_parameters_.end())
					{
						Log.error()<<"No value specified for '"<<current_par_name<<"', but it is a parameter not a flag!!\nUse '-help' to display information about available parameters and flags."<<endl;
					}
					else
					{
						Log.error()<<"Flag '"<<current_par_name<<"' unknown!!\nUse '-help' to display information about available parameters and flags."<<endl;
					}
					exit(1);
				}
			}
		}
		else
		{
			if (name_read) // command line parameter
			{
				replaceExcapedCharacters_(token);

				map<String, list<String> >::iterator it = parameter_map_.find(current_par_name);
				if (it != parameter_map_.end())
				{
					it->second.push_back(token);
				}
				else
				{
					if (registered_parameters_.find(current_par_name) != registered_parameters_.end())
					{
						list<String> v;
						v.push_back(token);
						parameter_map_.insert(make_pair(current_par_name, v));
					}
					else
					{
						Log.error()<<"Parameter '"<<current_par_name<<"' unknown!!\nUse '-help' to display information about available parameters and flags."<<endl;
						exit(1);
					}
				}
			}
			else
			{
				map<String, list<String> >::iterator it = parameter_map_.find(current_par_name);
				if (it != parameter_map_.end())
				{
					it->second.push_back(token);
				}
			}
			name_read = false;
		}
	}
	// In case last token is a flag ...
	if (name_read)
	{
		if (registered_flags_.find(current_par_name) != registered_flags_.end())
		{
			list<String> v;
			v.push_back("1");
			parameter_map_.insert(make_pair(current_par_name, v));
		}
		else
		{
			if (registered_parameters_.find(current_par_name) != registered_parameters_.end())
			{
				Log.error()<<"No value specified for '"<<current_par_name<<"', but it is a parameter not a flag!!\nUse '-help' to display information about available parameters and flags."<<endl;
			}
			else
			{
				Log.error()<<"Flag '"<<current_par_name<<"' unknown!!\nUse '-help' to display available parameters and flags."<<endl;
			}
			exit(1);
		}
	}


	// Do not complain about missing parameters if the user just wants to see the help page.
	if (parameter_map_.find("help") != parameter_map_.end())
	{
		printHelp();
		exit(0);
	}

	// Do not complain about missing parameters if the user just wants to write ini-file w/ default values.
	if (parameter_map_.find("write_ini") != parameter_map_.end())
	{
		return;
	}

	String par = get("par");
	String write_par = get("write_par");
	if (par != NOT_FOUND && par != "")
	{
		ParamFile pf(par, ios::in);
		list<pair<String, ParameterDescription> > descriptions;
		String par_toolname = "";
		String par_description = "";
		String par_version = "";
		String par_helptext = "";
		String category = "";
		pf.readSection(par_toolname, par_description, par_version, par_helptext, category, descriptions, parameter_map_, false);

		if (par_toolname != tool_name_)
		{
			Log.error()<<"[Error:] The specified parameter-file was created for tool '"<<par_toolname<<"' not for '"<<tool_name_<<"'. Please make sure to use the correct type of parameter-file!"<<endl;
			exit(1);
		}
		pf.close();
	}

	bool write_par_file = false;
	if (write_par != NOT_FOUND && write_par != "")
	{
		write_par_file = true;
	}

	// If no value for a parameter was specified on the command-line, but the parameter has a default-value, then make sure to use that default-value.
	for (map<String, list<String> >::iterator it = default_values.begin();
		it != default_values.end(); it++)
	{
		map<String, list<String> >::iterator search_it = parameter_map_.find(it->first);
		if (search_it == parameter_map_.end() || search_it->second.empty())
		{
			// Default values for flags would make no sense, so ignore them here
			if (!write_par_file && registered_flags_.find(it->first) != registered_flags_.end())
			{
				continue;
			}

			// Mandatory parameters by definition have to be specified ...
			if (!write_par_file && registered_parameters_.find(it->first)->second.mandatory)
			{
				continue;
			}

			parameter_map_[it->first] = it->second;
		}
	}

	if (write_par_file)
	{
		ParamFile pf(write_par, ios::out);
		parameter_map_.erase("write_par");
		list<pair<String, ParameterDescription> > descriptions;
		for (list<MapIterator>::iterator it = original_parameter_order_.begin();
			it != original_parameter_order_.end(); it++)
		{
			descriptions.push_back(make_pair((*it)->first, (*it)->second));
		}
		for (list<MapIterator>::iterator it = original_flag_order_.begin();
			it != original_flag_order_.end(); it++)
		{
			descriptions.push_back(make_pair((*it)->first, (*it)->second));
		}
		pf.writeSection(tool_name_, tool_description_, tool_version_, tool_manual_, tool_category_, descriptions, parameter_map_);
		pf.close();
		Log << "Tool-parameter file has been written to '"<<write_par<<"'. Goodbye!"<<endl;
		exit(0);
	}

	set<String> missing_parameters;
	for (map < String, ParameterDescription > :: iterator it = registered_parameters_.begin(); it != registered_parameters_.end(); it++)
	{
		if (it->second.mandatory == false) continue;
		if (parameter_map_.find(it->second.name) == parameter_map_.end())
		{
			missing_parameters.insert(it->first);
		}
	}
	if (missing_parameters.size() > 0)
	{
		Log.error() << "[Error:] The following mandatory parameters are missing:" << endl;
		printHelp(&missing_parameters, false);
		exit(1);
	}
}


void CommandlineParser::replaceExcapedCharacters_(String& parameter_value)
{
	for (list<pair<String, String> >::iterator it = excaped_chars_.begin(); it != excaped_chars_.end(); it++)
	{
		parameter_value.substituteAll(it->first, it->second);
	}
}


void CommandlineParser::copyAdvancedParametersToOptions(Options& options)
{
	for (list<MapIterator>::iterator it = original_parameter_order_.begin(); it != original_parameter_order_.end(); it++)
	{
		ParameterDescription& p = (*it)->second;

		if (!p.advanced) continue;

		map<String, list<String> >::iterator search_it = parameter_map_.find(p.name);
		if (search_it != parameter_map_.end())
		{
			Options* option_category = &options;
			if (p.category != "")
			{
				option_category = options.createSubcategory(p.category);
			}
			option_category->set(p.name, search_it->second.front());
		}
	}
}


void CommandlineParser::printHelp(set<String>* parameter_names, bool show_manual)
{
	if (!parameter_names && registered_parameters_.size() > 0)
	{
		cout<<"Available parameters are ('*' indicates mandatory parameters): "<<endl;
	}

	for (list<MapIterator>::iterator it = original_parameter_order_.begin(); it != original_parameter_order_.end(); it++)
	{
		ParameterDescription& p = (*it)->second;
		if ((parameter_names && parameter_names->find(p.name) == parameter_names->end()) || p.advanced)
		{
			continue;
		}
		cout<<"   ";
		if (!parameter_names && p.mandatory) cout<<"*  ";
		else cout<<"   ";
		cout<<"-"<<p.name;
		String n = "";
		if (p.type == INFILE)
		{
			n=" <in.file>";
		}
		else if (p.type == OUTFILE)
		{
			n=" <out.file>";
		}
		else if (p.type == BALL::INT)
		{
			n=" <int>";
		}
		else if (p.type == DOUBLE)
		{
			n=" <double>";
		}
		else if (p.type == STRING)
		{
			n=" <string>";
		}
		cout<<n;
		Size space_size = max_parname_length_+4-p.name.size()-n.size();
		for (Size j = 0; j < space_size; j++)
		{
			cout<<" ";
		}
		cout<<p.description<<endl;
	}
	if (!parameter_names && registered_flags_.size() > 0)
	{
		cout<<endl<<"Available flags are: "<<endl;
	}
	for (list<MapIterator>::iterator it = original_flag_order_.begin(); it != original_flag_order_.end(); it++)
	{
		ParameterDescription& p = (*it)->second;
		if (parameter_names && parameter_names->find(p.name) == parameter_names->end())
		{
			continue;
		}
		cout<<"      "<<"-"<<p.name;
		Size space_size = max_flagname_length_+4-p.name.size();
		for (Size j = 0; j < space_size; j++)
		{
			cout<<" ";
		}
		cout<<p.description<<endl;
	}
	cout<<endl;
	if (show_manual && tool_manual_ != "")
	{
		cout<<endl<<tool_manual_<<endl<<endl<<endl;
	}
}


const String& CommandlineParser::get(String name)
{
	map<String, list<String> >::iterator it = parameter_map_.find(name);
	if (it != parameter_map_.end())
	{
		return it->second.front();
	}
	else
	{
		return NOT_FOUND;
	}
}


bool CommandlineParser::has(String name)
{
	if (get(name) != NOT_FOUND)
	{
		return true;
	}
	else
	{
		return false;
	}
}


const list<String>& CommandlineParser::getList(String name)
{
	map<String, list<String> >::iterator it = parameter_map_.find(name);
	if (it != parameter_map_.end())
	{
		return it->second;
	}
	else
	{
		return EMTPY_LIST;
	}
}

const String& CommandlineParser::getStartTime()
{
	return start_time_;
}

void CommandlineParser::setToolManual(const String& manual)
{
	tool_manual_ = manual;
}

const String& CommandlineParser::getStartCommand()
{
	return start_command_;
}