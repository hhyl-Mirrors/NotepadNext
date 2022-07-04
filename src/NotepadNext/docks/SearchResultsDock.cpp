/*
 * This file is part of Notepad Next.
 * Copyright 2022 Justin Dailey
 *
 * Notepad Next is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Notepad Next is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Notepad Next.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "SearchResultsDock.h"
#include "ScintillaNext.h"
#include "ui_SearchResultsDock.h"

#include <QKeyEvent>
#include <QPointer>

SearchResultsDock::SearchResultsDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SearchResultsDock)
{
    ui->setupUi(this);

    connect(ui->treeWidget, &QTreeWidget::itemActivated, this, &SearchResultsDock::itemActivated);
}

SearchResultsDock::~SearchResultsDock()
{
    delete ui;
}

void SearchResultsDock::newSearch(const QString searchTerm)
{
    this->searchTerm = searchTerm;

    ui->treeWidget->collapseAll();

    currentSearch = new QTreeWidgetItem(ui->treeWidget);
    currentSearch->setBackground(0, QColor(232, 232, 255));
    currentSearch->setForeground(0, QColor(0, 0, 170));
    currentSearch->setExpanded(true);
    currentSearch->setFirstColumnSpanned(true);

    updateSearchStatus();
}

void SearchResultsDock::newFileEntry(ScintillaNext *editor)
{
    // Store a QPointer since there is no guarentee this editor will be around later
    QPointer<ScintillaNext> editor_pointer = editor;

    totalFileHitCount = 0;
    currentFilePath = editor->isFile() ? editor->getFilePath() : editor->getName();

    currentFile = new QTreeWidgetItem(currentSearch);
    currentFile->setData(0, Qt::UserRole, QVariant::fromValue(editor_pointer));

    currentFile->setBackground(0, QColor(213, 255, 213));
    currentFile->setForeground(0, QColor(0, 128, 0));
    currentFile->setExpanded(true);
    currentFile->setFirstColumnSpanned(true);

    currentFileCount++;
    updateSearchStatus();
}

void SearchResultsDock::newResultsEntry(const QString line, int lineNumber)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(currentFile);

    item->setText(0, QString::number(lineNumber));
    item->setData(0, Qt::UserRole, lineNumber);
    item->setBackground(0, QBrush(QColor(220, 220, 220)));
    item->setTextAlignment(0, Qt::AlignRight);

    item->setText(1, line);

    totalFileHitCount++;
    totalHitCount++;

    updateSearchStatus();
}

void SearchResultsDock::completeSearch()
{
    currentSearch = Q_NULLPTR;
    currentFile = Q_NULLPTR;
    currentFileCount = 0;
    totalFileHitCount = 0;
    totalHitCount = 0;

    ui->treeWidget->resizeColumnToContents(0);
}

void SearchResultsDock::itemActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    // Result entries have no children
    // Make sure the entry has a parent since search entries can have no children
    if (item->childCount() == 0 && item->parent() != Q_NULLPTR) {
        QPointer<ScintillaNext> editor = item->parent()->data(0, Qt::UserRole).value<QPointer<ScintillaNext>>();

        // The editor may no longer exist
        if (editor) {
            int lineNumber = item->data(0, Qt::UserRole).toInt();

            emit searchResultActivated(editor, lineNumber);
        }
    }
}

void SearchResultsDock::updateSearchStatus()
{
    currentSearch->setText(0, QStringLiteral("Search \"%1\" (%2 hits in %3 file)").arg(searchTerm).arg(totalHitCount).arg(currentFileCount));

    if (currentFile)
        currentFile->setText(0, QStringLiteral("%1 (%2 hits)").arg(currentFilePath).arg(totalFileHitCount));
}
