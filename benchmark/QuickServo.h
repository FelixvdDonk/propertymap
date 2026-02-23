#pragma once

#include <QElapsedTimer>
#include "../QuickPropertyMap/QuickPropertyMap.h"

/**
 * QuickPropertyMap approach: 200 properties created dynamically in a loop.
 * This replaces ~841 lines of StaticServo.h with ~50 lines.
 */
class QuickServo : public QuickPropertyMap
{
public:
    explicit QuickServo(QObject *parent = nullptr)
        : QuickPropertyMap(parent)
    {
        // Create all 200 servo properties in a loop
        for (int i = 0; i < 200; ++i)
            addProperty("servo" + QByteArray::number(i), 0.0f);
        build();

        // Cache indices for O(1) updates — no hash lookup per frame
        for (int i = 0; i < 200; ++i)
            m_idx[i] = indexOf("servo" + QByteArray::number(i));
    }

    /// Update all 200 properties at once (simulates a PLC cycle)
    void updateAll(const float *values)
    {
        for (int i = 0; i < 200; ++i)
            insert(m_idx[i], QVariant(values[i]));
    }

    /// Benchmark: update all properties N times, return total nanoseconds
    qint64 benchmark(int iterations)
    {
        float vals[200];
        for (int i = 0; i < 200; ++i) vals[i] = 0.0f;
        QElapsedTimer t;
        t.start();
        for (int n = 0; n < iterations; ++n) {
            for (int i = 0; i < 200; ++i) vals[i] = float(n * 200 + i);
            updateAll(vals);
        }
        return t.nsecsElapsed();
    }

private:
    int m_idx[200] = {};
};
