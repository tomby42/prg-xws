#include <qapplication.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qfont.h>
#include <qvbox.h>
#include <qhbox.h>

class MyWidget : public QVBox
{
public:
    MyWidget(QWidget *parent = 0, const char *name = 0);
};

MyWidget::MyWidget(QWidget *parent, const char *name) : QVBox(parent, name)
{
    QFont font("Times", 18, QFont::Bold);
    
    QPushButton *quit = new QPushButton("Quit", this, "quit");
    quit->setFont(font);

    QColor c("red");
    QLCDNumber *lcd = new QLCDNumber(8, this, "lcd");
    lcd->setSegmentStyle(QLCDNumber::Filled);
    lcd->setMode(QLCDNumber::HEX);
    lcd->setPaletteForegroundColor(c);

    QHBox *hbox = new QHBox(this, "hbox");
    QPushButton *b = new QPushButton("BINARY", hbox, "bin");
    b->setFont(font);
    b = new QPushButton("OCTADECIMAL", hbox, "oct");
    b->setFont(font);
    b = new QPushButton("DECIMAL", hbox, "dec");
    b->setFont(font);
    b = new QPushButton("HEXADECIMAL", hbox, "hex");
    b->setFont(font);
    
    QSlider *slider = new QSlider(Horizontal, this, "slider");
    slider->setRange(0, 255);
    slider->setValue(0);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MyWidget w;
    app.setMainWidget(&w);
    w.show();
    return app.exec();
}
