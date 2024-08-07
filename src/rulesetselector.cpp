/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rulesetselector.h"

#include "ruleset.h"
#include "rulesetdetailsdialog.h"
#include "settings.h"

#include <KLineEdit>
#include <KLocalizedString>

#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>

Killbots::RulesetSelector::RulesetSelector(QWidget *parent)
    : QWidget(parent),
      m_rulesetMap(),
      m_detailsDialog(nullptr)
{
    // Create a hidden KLineEdit to use the automatic KConfigXT connection
    kcfg_Ruleset = new KLineEdit();
    kcfg_Ruleset->setObjectName(QStringLiteral("kcfg_Ruleset"));
    kcfg_Ruleset->hide();

    m_listWidget = new QListWidget();
    m_listWidget->setWhatsThis(i18nc("@info:whatsthis", "A list of the Killbots rulesets installed on this computer."));

    QGroupBox *groupBox = new QGroupBox(i18nc("title:group", "Game Type Details"));
    groupBox->setWhatsThis(i18nc("@info:whatsthis", "Lists information on the currently selected game type."));

    QLabel *authorLabel = new QLabel(i18nc("@label", "Author:"));
    authorLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    m_author = new QLabel();
    m_author->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_author->setWordWrap(true);

    QLabel *contactLabel = new QLabel(i18nc("@label", "Contact:"));
    contactLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    m_authorContact = new QLabel();
    m_authorContact->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_authorContact->setWordWrap(true);
    m_authorContact->setOpenExternalLinks(true);

    QLabel *descriptionLabel = new QLabel(i18nc("@label", "Description:"));
    descriptionLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    m_description = new QLabel();
    m_description->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_description->setWordWrap(true);

    auto *detailsButton = new QPushButton(i18nc("@action:button", "Details…"));
    detailsButton->setToolTip(i18nc("@info:tooltip", "Show the detailed parameters of the selected game type"));
    detailsButton->setWhatsThis(i18nc("@info:whatsthisp", "Opens a dialog listing the values of all internal parameters for the selected game type."));

    QGridLayout *boxLayout = new QGridLayout(groupBox);
    boxLayout->addWidget(authorLabel, 1, 0);
    boxLayout->addWidget(m_author, 1, 1);
    boxLayout->addWidget(contactLabel, 2, 0);
    boxLayout->addWidget(m_authorContact, 2, 1);
    boxLayout->addWidget(descriptionLabel, 3, 0);
    boxLayout->addWidget(m_description, 3, 1);
    boxLayout->addWidget(detailsButton, 4, 1, Qt::AlignLeft);
    boxLayout->setColumnStretch(1, 10);
    boxLayout->setRowStretch(5, 10);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(kcfg_Ruleset);
    layout->addWidget(m_listWidget);
    layout->addWidget(groupBox, 10);

    connect(m_listWidget, &QListWidget::currentTextChanged, this, &RulesetSelector::selectionChanged);
    connect(detailsButton, &QPushButton::clicked, this, &RulesetSelector::showDetailsDialog);

    findRulesets();
}

Killbots::RulesetSelector::~RulesetSelector()
{
    qDeleteAll(m_rulesetMap);
}

void Killbots::RulesetSelector::findRulesets()
{
    qDeleteAll(m_rulesetMap);
    m_rulesetMap.clear();

    m_listWidget->clear();
    m_listWidget->setSortingEnabled(true);

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("killbots/rulesets/"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.desktop"));
        for (const QString &file : fileNames) {
            const Ruleset *ruleset = Ruleset::load(file);
            if (ruleset) {
                QString name = ruleset->name();
                while (m_rulesetMap.contains(name)) {
                    name += QLatin1Char('_');
                }

                m_rulesetMap.insert(name, ruleset);

                QListWidgetItem *item = new QListWidgetItem(name, m_listWidget);
                if (file == Settings::ruleset()) {
                    m_listWidget->setCurrentItem(item);
                }
            } else {
                delete ruleset;
            }
        }
    }

    // Set the maximum height of the list widget to be no more than the size of its contents
    // This is slightly hackish, but the effect is nice.
    const int itemHeight = m_listWidget->visualItemRect(m_listWidget->item(0)).height();
    const int verticalMargin = m_listWidget->height() - m_listWidget->viewport()->height();
    m_listWidget->setMaximumHeight(itemHeight * m_listWidget->count() + verticalMargin);
}

void Killbots::RulesetSelector::selectionChanged(const QString &rulesetName)
{
    const Ruleset *ruleset = m_rulesetMap.value(rulesetName);

    kcfg_Ruleset->setText(ruleset->fileName());

    m_author->setText(ruleset->author());
    if (ruleset->authorContact().contains(QLatin1Char(' '))) {
        m_authorContact->setText(ruleset->authorContact());
    } else if (ruleset->authorContact().contains(QLatin1Char('@'))) {
        m_authorContact->setText(QStringLiteral("<qt><a href=\"mailto:%1\">%1</a></qt>").arg(ruleset->authorContact()));
    } else if (ruleset->authorContact().contains(QLatin1Char('.'))) {
        m_authorContact->setText(QStringLiteral("<qt><a href=\"https://%1\">%1</a></qt>").arg(ruleset->authorContact()));
    } else {
        m_authorContact->setText(ruleset->authorContact());
    }
    m_description->setText(ruleset->description());

    if (m_detailsDialog && m_detailsDialog->isVisible()) {
        m_detailsDialog->loadRuleset(ruleset);
    }
}

void Killbots::RulesetSelector::showDetailsDialog()
{
    if (!m_detailsDialog) {
        m_detailsDialog = new RulesetDetailsDialog(this);
    }

    const Ruleset *ruleset = m_rulesetMap.value(m_listWidget->currentItem()->text());
    m_detailsDialog->loadRuleset(ruleset);
    m_detailsDialog->show();
}

#include "moc_rulesetselector.cpp"
