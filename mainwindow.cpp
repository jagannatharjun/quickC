#include "mainwindow.h"
#include "cppsyntaxhightlighter.h"
#include "editprocess.h"
#include "sourcecodeeditor.h"
#include "ui_mainwindow.h"
#include <QAction>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>
#include <cstdlib>

struct MainWindow::_Detail {
  SourceCodeEditor sourceEdit;
  QVBoxLayout centralWidgetVLayout;
  EditProcess compilationEdit, runEdit;
  QTabWidget runMenuTabs;
  CppSyntaxHightlighter highlighter;
  _Detail() : highlighter{sourceEdit.document()} {
    compilationEdit.setArguments({"-x", "c", "-Wall", "-"});
    qputenv("path", qgetenv("path") + ";./Mingw/bin/");
    if (QFile("./Mingw/bin/gcc.exe").exists())
      compilationEdit.setProgram("./Mingw/bin/gcc.exe");
    else
      compilationEdit.setProgram("gcc");
    qDebug() << "Compiler exe " << compilationEdit.program();
  }

  void compileSrcEdit();

public:
  void run();

private:
  void parseCompilerOutput(std::vector<CompilerMsgs> &result) {
    result.clear();
    QString o = compilationEdit.edit()->toPlainText();
    o.chop(o.size() - o.lastIndexOf('\n')); // remove last \nProgram Return...
    qDebug() << "Parsing " << o;
    auto extractInt = [](auto b) {
      int d = 0;
      for (; b->isDigit(); b++)
        d = d * 10 + b->digitValue();
      return std::make_pair(b, d);
    };
    for (auto b = o.cbegin(); b != o.cend(); ++b) {
      CompilerMsgs back;
      while (*b++ != ':')
        if (b == o.cend())
          return;
      auto r = extractInt(b);
      b = r.first, back.lineNo = r.second;
      b++;
      r = extractInt(b);
      b = r.first, back.columnNo = r.second;
      b++;
      while (b != o.cend() && *b != '\n')
        back.message.push_back(*b++);
      back.message = back.message.trimmed(); // remove extra spaces
      back.msgSeverity = back.message.startsWith("error")
                             ? CompilerMsgs::Severity::Error
                             : back.message.startsWith("warning")
                                   ? CompilerMsgs::Severity::Warning
                                   : CompilerMsgs::Severity::Unknown;
      if (back.msgSeverity == CompilerMsgs::Severity::Unknown)
        continue;
      result.emplace_back(std::move(back));
    }
  }
};

void MainWindow::arrangeCentralWidgetElements() {
  centralWidget()->setLayout(&details->centralWidgetVLayout);

  details->runMenuTabs.addTab(details->compilationEdit.edit(), "Compilation");
  details->runMenuTabs.addTab(details->runEdit.edit(), "Run");
  details->runMenuTabs.setStyleSheet("margin: 5px");

  auto centralSplitter = new QSplitter(Qt::Vertical);
  centralSplitter->addWidget(&details->sourceEdit);
  centralSplitter->addWidget(&details->runMenuTabs);
  centralSplitter->setStretchFactor(0, 4);
  centralSplitter->setSizes({1000, 200});
  details->centralWidgetVLayout.addWidget(centralSplitter);
}

void MainWindow::setShortCuts() {
  ui->actionOpen->setShortcut(QKeySequence::Open);
  ui->actionSave->setShortcut(QKeySequence::Save);
  ui->actionZoom_In->setShortcut(QKeySequence::ZoomIn);
  ui->actionZoom_Out->setShortcut(QKeySequence::ZoomOut);
  ui->actionCompile->setShortcut(QKeySequence("F2"));
  ui->actionCompile_And_Run->setShortcut(QKeySequence("Ctrl+R"));
  ui->actionRun->setShortcut(QKeySequence("Ctrl+Shift+R"));
}

void MainWindow::setMenuEdit() {
  connect(ui->menuEdit, &QMenu::triggered, [this](QAction *action) {
    if (action == ui->actionZoom_In)
      details->sourceEdit.zoomIn();
    else if (action == ui->actionZoom_Out)
      details->sourceEdit.zoomOut();
  });
}

void MainWindow::setMenuCompile() {
  connect(ui->menuRun, &QMenu::triggered, [this](QAction *action) {
    if (action == ui->actionCompile)
      details->compileSrcEdit();
    else if (action == ui->actionRun)
      details->run();
    else if (action == ui->actionCompile_And_Run) {
      details->compileSrcEdit();
      if (!details->compilationEdit.exitCode()) // compilation is success
        details->run();
    }
  });
}

QString getGlobalStyleSheet(const QColor &baseColor, const QColor &);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow), details{std::make_unique<_Detail>()} {
  ui->setupUi(this);
  arrangeCentralWidgetElements();

  details->sourceEdit.document()->setPlainText(
      "#include <stdio.h>\n\nint main() {\n\tprintf(\"Hello World\");\n}");

  setStyleSheet(
      getGlobalStyleSheet(QColor("#3c3c3c").darker(), QColor("#2c2c2c")));
  //  details->sourceEdit.document()->setPlainText(
  //      );
  //  setStyleSheet(details->sourceEdit.toPlainText());
  //  auto styleSheetAction = new QAction(this);
  //  styleSheetAction->setShortcut(QKeySequence("F3"));
  //  connect(styleSheetAction, &QAction::triggered, [this](bool = false) {
  //    qDebug() << "setting StyleSheet";
  //    setStyleSheet(details->sourceEdit.toPlainText());
  //  });
  //  addAction(styleSheetAction);

  connect(ui->menuFile, &QMenu::triggered, this,
          &MainWindow::menuFileTriggered);

  setMenuCompile();
  setMenuEdit();
  setShortCuts();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::menuFileTriggered(QAction *action) {
  if (action == ui->actionOpen) {
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "./",
                                                 "All Files(*)");
    if (fileName.isEmpty())
      return;
    details->sourceEdit.loadFile(fileName);
  } else if (action == ui->actionSave) {
    auto fileName = QFileDialog::getSaveFileName(this, tr("Open File"), "./",
                                                 "All Files(*)");
    if (fileName.isEmpty())
      return;
    details->sourceEdit.saveFile(fileName);
  }
}

void MainWindow::_Detail::run() {
  runEdit.setProgram("./a");
  runEdit.edit()->setPlainText("");
  runMenuTabs.setCurrentWidget(runEdit.edit());
  runEdit.start();
}

void MainWindow::_Detail::compileSrcEdit() {
  QString src = sourceEdit.document()->toPlainText();
  qDebug() << "Compiling " << src;
  compilationEdit.edit()->setPlainText("");
  runMenuTabs.setCurrentWidget(compilationEdit.edit());
  compilationEdit.start();
  if (!compilationEdit.waitForStarted()) {
    qDebug() << "Failed to start GCC";
  }
  compilationEdit.write(src.toUtf8());
  compilationEdit.closeWriteChannel();
  while (!compilationEdit.waitForFinished())
    ;
  sourceEdit.setCompilerMsgs(
      std::bind(&_Detail::parseCompilerOutput, this, std::placeholders::_1));
}
