#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>

static QLabel *cfgLabel(QLabel *l)
{
    l->setMargin(10);
    l->setFrameShape(QFrame::Panel);
    l->setFrameShadow(QFrame::Sunken);
    l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    return l;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    window.setCaption("Layouts");
    QGridLayout *layout = new QGridLayout(&window, 2, 3, 10, 20);
    QLabel *l = new QLabel("<B>QHBox</B>", &window);
    l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(l, 0, 0);
    l = new QLabel("<B>QVBox</B>", &window);
    l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(l, 0, 1);
    l = new QLabel("<B>QGrid</B>", &window);
    l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(l, 0, 2);

    QHBoxLayout *hbox = new QHBoxLayout(0);
    layout->addLayout(hbox, 1, 0);
    hbox->addWidget(cfgLabel(new QLabel("1", &window)));
    hbox->addWidget(cfgLabel(new QLabel("2", &window)));
    hbox->addWidget(cfgLabel(new QLabel("3", &window)));

    QVBoxLayout *vbox = new QVBoxLayout(0);
    layout->addLayout(vbox, 1, 1);
    vbox->addWidget(cfgLabel(new QLabel("1", &window)));
    vbox->addWidget(cfgLabel(new QLabel("2", &window)));
    vbox->addWidget(cfgLabel(new QLabel("3", &window)));

    QGridLayout *grid = new QGridLayout(3, 2, 0);
    layout->addLayout(grid, 1, 2);
    grid->addWidget(cfgLabel(new QLabel("1", &window)), 0, 0);
    grid->addWidget(cfgLabel(new QLabel("2", &window)), 0, 1);
    grid->addWidget(cfgLabel(new QLabel("3", &window)), 1, 0);
    grid->addWidget(cfgLabel(new QLabel("4", &window)), 1, 1);
    grid->addWidget(cfgLabel(new QLabel("5", &window)), 2, 0);

    app.setMainWidget(&window);
    window.show();
    return app.exec();
}
