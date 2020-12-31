/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rulesetdetailsdialog.h"

#include "ruleset.h"

#include <KLocalizedString>

#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

Killbots::RulesetDetailsDialog::RulesetDetailsDialog(QWidget *parent)
    : QDialog(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mMainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mMainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RulesetDetailsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RulesetDetailsDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void Killbots::RulesetDetailsDialog::loadRuleset(const Ruleset *ruleset)
{
    static QStringList maskedItems = QStringList() << QStringLiteral("Name") << QStringLiteral("Author") << QStringLiteral("AuthorContact") << QStringLiteral("Description");
    const QStringList junkheapEnumText {
	i18nc("@item Quantity of junkheaps that can be pushed", "None"),
        i18nc("@item Quantity of junkheaps that can be pushed", "One"),
        i18nc("@item Quantity of junkheaps that can be pushed", "Many"),
    };

    // If the dialog hasn't been setup already, do so.
    if (m_labels.size() == 0) {
        QFormLayout *layout = new QFormLayout(mMainWidget);

        const auto items = ruleset->items();
        for (const KConfigSkeletonItem *item : items) {
            if (!maskedItems.contains(item->name())) {
                QString labelText = item->label().isEmpty() ? item->name() : item->label();
                labelText = i18nc("%1 is a pretranslated string that we're turning into a label", "%1:", labelText);

                QLabel *valueLabel = new QLabel();
                m_labels[ item->name() ] = valueLabel;
                layout->addRow(new QLabel(labelText), valueLabel);
            }
        }

    }

    QMap<QString, QLabel *>::const_iterator it = m_labels.constBegin();
    QMap<QString, QLabel *>::const_iterator end = m_labels.constEnd();
    for (; it != end; it++) {
        const KConfigSkeletonItem *item = ruleset->findItem(it.key());

        QString valueText;
        if (dynamic_cast<const KCoreConfigSkeleton::ItemBool *>(item)) {
            valueText = item->property().toBool() ? i18nc("@item", "Yes") : i18nc("@item", "No");
        } else if (it.key() == QLatin1String("PushableJunkheaps")) {
            valueText = junkheapEnumText.at(item->property().toInt());
        } else {
            valueText = item->property().toString();
        }

        it.value()->setText(valueText);
    }

    setWindowTitle(i18nc("@title:window", "Details of %1 Game Type", ruleset->name()));
}

