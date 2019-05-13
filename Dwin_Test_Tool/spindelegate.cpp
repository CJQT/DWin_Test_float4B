#include "spindelegate.h"
#include <QSpinBox>

SpinDelegate::SpinDelegate(QObject *parent)
    : QItemDelegate(parent)
{

}

QWidget* SpinDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    decltype(index.column()) col = index.column();
    if(col == 0 || col == 4)
    {
        return NULL;
    }
    //QSpinBox *editor = new QSpinBox(parent);
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    //editor->setDecimals(3);
    editor->setRange(-100.000,32767.000);
    editor->installEventFilter(const_cast<SpinDelegate*>(this));
    if(col == 2 || col == 6)
    {
        editor->setDecimals(2);
    }
    else
    {
        editor->setDecimals(3);
    }

    return editor;
}

void SpinDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
{
    float value = index.model()->data(index).toFloat();
    QDoubleSpinBox *box = static_cast<QDoubleSpinBox*>(editor);
    box->setValue(value);
}
void SpinDelegate::setModelData(QWidget *editor,QAbstractItemModel *model,const QModelIndex &index) const
{
    QDoubleSpinBox *box = static_cast<QDoubleSpinBox*>(editor);
    float value = box->value();
    model->setData(index,value);
}

void SpinDelegate::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem *option,
                          const QModelIndex &/*index*/) const
{
    editor->setGeometry(option->rect);
}
