#include <qapplication.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qlabel.h>

static void cfgLabel(QLabel *l)
{
    l->setMargin(10);
    l->setFrameShape(QFrame::Panel);
    l->setFrameShadow(QFrame::Sunken);
    l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGrid window(3);
    window.setCaption("Layout widgets");
    window.setSpacing(20);
    window.setMargin(10);
    (new QLabel("<B>QHBox</B>", &window))->
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    (new QLabel("<B>QVBox</B>", &window))->
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    (new QLabel("<B>QGrid</B>", &window))->
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QHBox *hbox = new QHBox(&window);
    cfgLabel(new QLabel("1", hbox));
    cfgLabel(new QLabel("2", hbox));
    cfgLabel(new QLabel("3", hbox));

    QHBox *vbox = new QVBox(&window);
    cfgLabel(new QLabel("1", vbox));
    cfgLabel(new QLabel("2", vbox));
    cfgLabel(new QLabel("3", vbox));

    QGrid *grid = new QGrid(2, &window);
    cfgLabel(new QLabel("1", grid));
    cfgLabel(new QLabel("2", grid));
    cfgLabel(new QLabel("3", grid));
    cfgLabel(new QLabel("4", grid));
    cfgLabel(new QLabel("5", grid));

    app.setMainWidget(&window);
    window.show();
    return app.exec();
}
