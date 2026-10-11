#include "Gui/ViewModels/MetricsViewModel.hpp"

#include <cmath>

namespace Sarwave::Core::Gui::ViewModels {

namespace {

// EqualityComparer<double>.Default: NaN equals NaN.
bool same(double a, double b) {
    return a == b || (std::isnan(a) && std::isnan(b));
}

}  // namespace

void MetricsViewModel::setRange(double value) {
    if (same(m_range, value)) return;
    m_range = value;
    emit rangeChanged();
}

void MetricsViewModel::setDoppler(double value) {
    if (same(m_doppler, value)) return;
    m_doppler = value;
    emit dopplerChanged();
}

void MetricsViewModel::setDensity(double value) {
    if (same(m_density, value)) return;
    m_density = value;
    emit densityChanged();
}

void MetricsViewModel::setSnr(double value) {
    if (same(m_snr, value)) return;
    m_snr = value;
    emit snrChanged();
}

}  // namespace Sarwave::Core::Gui::ViewModels
