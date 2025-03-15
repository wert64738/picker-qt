#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QScreen>
#include <QPixmap>
#include <QPainter>
#include <QCursor>
#include <QKeyEvent>
#include <QDebug>
#include <QLineEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QImage>
#include <QString>
#include <QColor>
#include <QFont>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        scaleFactor = 4;
        liveViewLocked = false;
        
        // Central widget and overall vertical layout.
        QWidget* centralWidget = new QWidget(this);
        centralWidget->setFocusPolicy(Qt::StrongFocus);
        setCentralWidget(centralWidget);
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(10);
        
        // Create a horizontal layout for the color preview and live view.
        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->setSpacing(10);
        
        // Color preview box
        colorLabel = new QLabel(centralWidget);
        const int captureSize = 11;
        const int totalZoomFactor = 16;
        int viewSize = captureSize * totalZoomFactor; // 176
        colorLabel->setFixedSize(viewSize, viewSize);
        colorLabel->setStyleSheet("background-color: #000000;");
        topLayout->addWidget(colorLabel);
        
        // Live view image label.
        imageLabel = new QLabel(centralWidget);
        imageLabel->setFixedSize(viewSize, viewSize);
        imageLabel->setAlignment(Qt::AlignCenter);
        topLayout->addWidget(imageLabel);
        
        mainLayout->addLayout(topLayout);
        
        // Form layout for text boxes below the view.
        QFormLayout* formLayout = new QFormLayout();
        QFont smallFont;
        smallFont.setPointSize(8);
        
        coordsLineEdit = new QLineEdit(centralWidget);
        coordsLineEdit->setReadOnly(true);
        coordsLineEdit->setFocusPolicy(Qt::NoFocus);
        coordsLineEdit->setMinimumWidth(250);
        coordsLineEdit->setFont(smallFont);
        formLayout->addRow("Coordinates:", coordsLineEdit);
        
        hexLineEdit = new QLineEdit(centralWidget);
        hexLineEdit->setReadOnly(true);
        hexLineEdit->setFocusPolicy(Qt::NoFocus);
        hexLineEdit->setMinimumWidth(250);
        hexLineEdit->setFont(smallFont);
        formLayout->addRow("HEX:", hexLineEdit);
        
        rgbLineEdit = new QLineEdit(centralWidget);
        rgbLineEdit->setReadOnly(true);
        rgbLineEdit->setFocusPolicy(Qt::NoFocus);
        rgbLineEdit->setMinimumWidth(250);
        rgbLineEdit->setFont(smallFont);
        formLayout->addRow("RGB:", rgbLineEdit);
        
        mainLayout->addLayout(formLayout);
        
        // Set overall window size (adjust margins as needed).
        int overallWidth = viewSize * 2 + 40;
        int overallHeight = viewSize + 120;
        setFixedSize(overallWidth, overallHeight);
        
        // Timer to update the view.
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::updateLiveView);
        timer->start(32);
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        QPoint pos = QCursor::pos();
        switch (event->key()) {
            case Qt::Key_Up:
                pos.setY(pos.y() - 1);
                QCursor::setPos(pos);
                break;
            case Qt::Key_Down:
                pos.setY(pos.y() + 1);
                QCursor::setPos(pos);
                break;
            case Qt::Key_Left:
                pos.setX(pos.x() - 1);
                QCursor::setPos(pos);
                break;
            case Qt::Key_Right:
                pos.setX(pos.x() + 1);
                QCursor::setPos(pos);
                break;
            case Qt::Key_F:
                liveViewLocked = !liveViewLocked;
                if (liveViewLocked) {
                    lockedPos = QCursor::pos();
                    qDebug() << "Live View Locked at" << lockedPos;
                } else {
                    qDebug() << "Live View Unlocked";
                }
                break;
            default:
                QMainWindow::keyPressEvent(event);
        }
    }

public slots:
    void updateLiveView() {
        // Use locked position if enabled.
        QPoint capturePos = liveViewLocked ? lockedPos : QCursor::pos();
        const int captureSize = 11;
        int half = captureSize / 2;
        int captureX = capturePos.x() - half;
        int captureY = capturePos.y() - half;

        QScreen *screen = QApplication::primaryScreen();
        if (!screen)
            return;
        QPixmap capture = screen->grabWindow(0, captureX, captureY, captureSize, captureSize);
        
        // Get center pixel's color.
        QImage captureImage = capture.toImage();
        QColor centerColor = captureImage.pixelColor(5, 5);
        
        // Update the color preview box.
        colorLabel->setStyleSheet(QString("background-color: %1;").arg(centerColor.name()));
        
        const int totalZoomFactor = 16;
        QPixmap zoomed = capture.scaled(captureSize * totalZoomFactor,
                                        captureSize * totalZoomFactor,
                                        Qt::KeepAspectRatio,
                                        Qt::FastTransformation);
        QPainter painter(&zoomed);
        painter.setPen(Qt::red);
        int rectX = 18 * scaleFactor;
        int rectY = 18 * scaleFactor;
        int rectWidth = 3 * scaleFactor;
        int rectHeight = 3 * scaleFactor;
        painter.drawRect(rectX, rectY, rectWidth, rectHeight);
        
        imageLabel->setPixmap(zoomed);
        
        // Update text fields.
        coordsLineEdit->setText(QString("X: %1, Y: %2").arg(capturePos.x()).arg(capturePos.y()));
        hexLineEdit->setText(centerColor.name().toUpper());
        rgbLineEdit->setText(QString("R: %1, G: %2, B: %3")
                             .arg(centerColor.red())
                             .arg(centerColor.green())
                             .arg(centerColor.blue()));
    }

private:
    QLabel *colorLabel;
    QLabel *imageLabel;
    QLineEdit *coordsLineEdit;
    QLineEdit *hexLineEdit;
    QLineEdit *rgbLineEdit;
    QTimer *timer;
    int scaleFactor;
    bool liveViewLocked;
    QPoint lockedPos;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.setWindowTitle("picker-qt");
    window.show();
    return app.exec();
}
