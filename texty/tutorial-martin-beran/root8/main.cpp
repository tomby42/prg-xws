// Qt Tutorial #1, kap. 7

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qvbox.h>
#include <qgrid.h>

#include "lcdrange.h"

class MyWidget : public QVBox {
public:
    MyWidget(QWidget *parent = 0, const char *name = 0);
};

MyWidget::MyWidget(QWidget *parent, const char *name) : QVBox(parent, name)
{
    QPushButton *quit = new QPushButton("Quit", this, "quit");
    connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()));

    LCDRange *prev = 0;
    QGrid *grid = new QGrid(4, this);
    for(int r = 0; r < 4; r++)
	for(int c = 0; c < 4; c++) {
	    LCDRange *lr = new LCDRange(grid);
	    if(prev)
		connect(lr, SIGNAL(valueChanged(int)),
			prev, SLOT(setValue(int)));
	    prev = lr;
	}
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MyWidget w;
    app.setMainWidget(&w);
    w.show();
    return app.exec();
}
