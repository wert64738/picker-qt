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

// Constants
constexpr int CAPTURE_SIZE      = 11;
constexpr int TOTAL_ZOOM_FACTOR = 16;
constexpr int VIEW_SIZE         = CAPTURE_SIZE * TOTAL_ZOOM_FACTOR; // 176
constexpr int SCALE_FACTOR      = 4;
constexpr int RECT_X            = 18 * SCALE_FACTOR;
constexpr int RECT_Y            = 18 * SCALE_FACTOR;
constexpr int RECT_WIDTH        = 3  * SCALE_FACTOR;
constexpr int RECT_HEIGHT       = 3  * SCALE_FACTOR;
constexpr int OVERALL_WIDTH     = VIEW_SIZE * 2 + 40;
constexpr int OVERALL_HEIGHT    = VIEW_SIZE + 140; // Increased height for extra spacing

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        liveViewLocked = false;
        
        // Central widget with overall vertical layout.
        QWidget* centralWidget = new QWidget(this);
        centralWidget->setFocusPolicy(Qt::StrongFocus);
        setCentralWidget(centralWidget);
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(10);
        
        // Top horizontal layout for the color preview and live view.
        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->setSpacing(10);
        
        // Color preview box on the left.
        colorLabel = new QLabel(centralWidget);
        colorLabel->setFixedSize(VIEW_SIZE, VIEW_SIZE);
        colorLabel->setStyleSheet("background-color: #000000;");
        topLayout->addWidget(colorLabel);
        
        // Live view image label.
        imageLabel = new QLabel(centralWidget);
        imageLabel->setFixedSize(VIEW_SIZE, VIEW_SIZE);
        imageLabel->setAlignment(Qt::AlignCenter);
        topLayout->addWidget(imageLabel);
        
        mainLayout->addLayout(topLayout);
        
        // Extra spacing to ensure text fields don't overlap the view boxes.
        mainLayout->addSpacing(VIEW_SIZE);
        
        // Form layout for text fields.
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
        
        setFixedSize(OVERALL_WIDTH, OVERALL_HEIGHT);
        
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
        QPoint capturePos = liveViewLocked ? lockedPos : QCursor::pos();
        int half = CAPTURE_SIZE / 2;
        int captureX = capturePos.x() - half;
        int captureY = capturePos.y() - half;

        QScreen *screen = QApplication::primaryScreen();
        if (!screen)
            return;
        QPixmap capture = screen->grabWindow(0, captureX, captureY, CAPTURE_SIZE, CAPTURE_SIZE);
        
        QImage captureImage = capture.toImage();
        QColor centerColor = captureImage.pixelColor(5, 5);
        
        colorLabel->setStyleSheet(QString("background-color: %1;").arg(centerColor.name()));
        
        QPixmap zoomed = capture.scaled(VIEW_SIZE,
                                        VIEW_SIZE,
                                        Qt::KeepAspectRatio,
                                        Qt::FastTransformation);
        QPainter painter(&zoomed);
        painter.setPen(Qt::red);
        painter.drawRect(RECT_X, RECT_Y, RECT_WIDTH, RECT_HEIGHT);
        
        imageLabel->setPixmap(zoomed);
        
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
