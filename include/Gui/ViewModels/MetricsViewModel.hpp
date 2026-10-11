#pragma once

#include "Gui/ViewModels/ViewModelBase.hpp"

namespace Sarwave::Core::Gui::ViewModels {

/// The four values of the bottom strip. Formatting (decimals) lives in MetricsStrip.qml.
class MetricsViewModel : public ViewModelBase {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the dashboard view models.")

    Q_PROPERTY(double range READ range WRITE setRange NOTIFY rangeChanged)          // m
    Q_PROPERTY(double doppler READ doppler WRITE setDoppler NOTIFY dopplerChanged)  // m/s
    Q_PROPERTY(double density READ density WRITE setDensity NOTIFY densityChanged)  // points
    Q_PROPERTY(double snr READ snr WRITE setSnr NOTIFY snrChanged)                  // dB

   public:
    using ViewModelBase::ViewModelBase;

    [[nodiscard]] double range() const {
        return m_range;
    }
    void setRange(double value);

    [[nodiscard]] double doppler() const {
        return m_doppler;
    }
    void setDoppler(double value);

    [[nodiscard]] double density() const {
        return m_density;
    }
    void setDensity(double value);

    [[nodiscard]] double snr() const {
        return m_snr;
    }
    void setSnr(double value);

   signals:
    void rangeChanged();
    void dopplerChanged();
    void densityChanged();
    void snrChanged();

   private:
    double m_range = 0;
    double m_doppler = 0;
    double m_density = 0;
    double m_snr = 0;
};

}  // namespace Sarwave::Core::Gui::ViewModels
