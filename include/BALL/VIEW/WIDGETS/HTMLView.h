#ifndef BALL_VIEW_WIDGETS_HTMLVIEW_H
#define BALL_VIEW_WIDGETS_HTMLVIEW_H

#ifndef BALL_VIEW_WIDGETS_DOCKWIDGET_H
	#include <BALL/VIEW/WIDGETS/dockWidget.h>
#endif

#include <QtCore/QUrl>
#include <QtWebKit/QWebView>

namespace BALL
{
	namespace VIEW
	{

		class HTMLView : public QWebView
		{
			Q_OBJECT

			public:
				HTMLView(QWidget* parent = 0);
		};

		class HTMLViewDock : public DockWidget
		{
			public:
				HTMLViewDock(QWidget* parent, const char* title = 0);
				HTMLViewDock(HTMLView* view,  QWidget* parent, const char* title = 0);

				void setHTMLView(HTMLView* view);
				HTMLView* getHTMLView();
				const HTMLView* getHTMLView() const;

			protected:
				HTMLView* html_view_;
		};

	}
}

#endif // BALL_VIEW_WIDGETS_HTMLVIEW_H