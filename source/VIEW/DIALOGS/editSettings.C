// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/VIEW/DIALOGS/editSettings.h>
#include <BALL/VIEW/KERNEL/common.h>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>

namespace BALL
{
	namespace VIEW
	{

EditSettings::EditSettings(QWidget* parent, const char* name, Qt::WindowFlags fl)
	: QWidget(parent, fl),
		Ui_EditSettingsData(),
		PreferencesEntry()
{
	setupUi(this);
	setObjectName(name);
	setINIFileSectionName("EDITING");
	setWidgetStackName((String)tr("Editing"));
	registerWidgets_();
}

EditSettings::~EditSettings()
{
	#ifdef BALL_VIEW_DEBUG
		Log.error() << "Destructing object " << (void *)this 
								<< " of class EditSettings" << std::endl;
	#endif 
}

} } // namespaces
