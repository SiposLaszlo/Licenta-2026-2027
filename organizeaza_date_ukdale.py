"""
Script pentru organizarea datelor UK-DALE (Household 3) pentru proiect NILM
- Citeste canalele 1-5 din date-UKDALE/house_3/
- Scrie datele in Excel (date prelucrate/Excel/UKDALE_Household3_Consumatori.xlsx)
  in formatul cerut: coloana, coloana, liber, coloana, coloana, liber... etc.
  cu coloana de consum total
- Genereaza grafice detaliate cu pandas si matplotlib in date prelucrate/grafice/
"""

import os
import time
from datetime import datetime
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import openpyxl

# Setare stil matplotlib pentru grafice profesionale de licenta
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams['font.sans-serif'] = 'DejaVu Sans'
plt.rcParams['axes.edgecolor'] = '#cccccc'
plt.rcParams['axes.linewidth'] = 0.8

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, 'date-UKDALE', 'house_3')
OUT_EXCEL_DIR = os.path.join(BASE_DIR, 'date prelucrate', 'Excel')
OUT_PLOTS_DIR = os.path.join(BASE_DIR, 'date prelucrate', 'grafice')

os.makedirs(OUT_EXCEL_DIR, exist_ok=True)
os.makedirs(OUT_PLOTS_DIR, exist_ok=True)

# 1. Citire etichete consumatori
labels = {}
labels_file = os.path.join(DATA_DIR, 'labels.dat')
if os.path.exists(labels_file):
    with open(labels_file, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line:
                parts = line.split()
                labels[int(parts[0])] = parts[1]
else:
    labels = {
        1: 'aggregate',
        2: 'kettle',
        3: 'electric_heater',
        4: 'laptop',
        5: 'projector'
    }

print("Etichete identificate:", labels)

# Mapare nume prietenoase in limba romana si descrieri
friendly_names = {
    1: 'Canal 1 - Consum Agregat Masurat (Aggregate)',
    2: 'Canal 2 - Fierbator (Kettle)',
    3: 'Canal 3 - Calorifer Electric (Electric Heater)',
    4: 'Canal 4 - Laptop',
    5: 'Canal 5 - Videoproiector (Projector)'
}

# 2. Citire fisiere .dat cu pandas
print("\nCitire fisiere .dat din UK-DALE House 3...")
t0 = time.time()
channels_data = {}
min_len = float('inf')
max_len = 0

for ch in range(1, 6):
    file_path = os.path.join(DATA_DIR, f'channel_{ch}.dat')
    df = pd.read_csv(file_path, sep=' ', header=None, names=['timestamp', 'power'], dtype={'timestamp': 'int64', 'power': 'float32'})
    channels_data[ch] = df
    cnt = len(df)
    if cnt < min_len:
        min_len = cnt
    if cnt > max_len:
        max_len = cnt
    print(f"  -> Canal {ch} ({labels.get(ch, 'necunoscut')}): {cnt:,} inregistrari, putere medie = {df['power'].mean():.2f} W, max = {df['power'].max():.1f} W")

print(f"Citire finalizata in {time.time() - t0:.2f} secunde.")

# Calcul consum total
# In NILM UK-DALE:
# - Canalul 1 este consumul total al intregii case masurat de contorul general (aggregate)
# - Canalele 2-5 sunt aparatele individuale monitorizate
# Calculam si suma aparatelor monitorizate (Canalele 2, 3, 4, 5)
common_rows = min(len(channels_data[2]), len(channels_data[3]), len(channels_data[4]), len(channels_data[5]))
sum_submeters = (
    channels_data[2]['power'].iloc[:common_rows].values +
    channels_data[3]['power'].iloc[:common_rows].values +
    channels_data[4]['power'].iloc[:common_rows].values +
    channels_data[5]['power'].iloc[:common_rows].values
)

# 3. Creare fisier Excel cu formatul solicitat:
# coloana, coloana, liber, coloana, coloana, liber... etc.
# cu coloana de Consum Total
excel_path = os.path.join(OUT_EXCEL_DIR, 'UKDALE_Household3_Consumatori.xlsx')
print(f"\nScriere fisier Excel: {excel_path}")
t_excel = time.time()

wb = openpyxl.Workbook(write_only=True)
ws = wb.create_sheet(title='Date_Consumatori')

# Antetul tabelului
header = [
    'Timp_Canal1_Aggregate (s)', 'Consum_Canal1_Aggregate (W)', None,
    'Timp_Canal2_Kettle (s)', 'Consum_Canal2_Kettle (W)', None,
    'Timp_Canal3_Heater (s)', 'Consum_Canal3_Heater (W)', None,
    'Timp_Canal4_Laptop (s)', 'Consum_Canal4_Laptop (W)', None,
    'Timp_Canal5_Projector (s)', 'Consum_Canal5_Projector (W)', None,
    'Consum_Total_Calculat_Suma_Aparate (W)',
    'Consum_Total_Masurat_Canal1_Aggregate (W)'
]
ws.append(header)

# Extragere vectori numpy pentru viteza maxima
ts1 = channels_data[1]['timestamp'].values
p1 = channels_data[1]['power'].values
n1 = len(ts1)

ts2 = channels_data[2]['timestamp'].values
p2 = channels_data[2]['power'].values
n2 = len(ts2)

ts3 = channels_data[3]['timestamp'].values
p3 = channels_data[3]['power'].values
n3 = len(ts3)

ts4 = channels_data[4]['timestamp'].values
p4 = channels_data[4]['power'].values
n4 = len(ts4)

ts5 = channels_data[5]['timestamp'].values
p5 = channels_data[5]['power'].values
n5 = len(ts5)

n_sum = len(sum_submeters)

# Scrierea randurilor
print(f"Scriere {max_len:,} randuri in Excel...")
for i in range(max_len):
    row = [
        int(ts1[i]) if i < n1 else None,
        float(p1[i]) if i < n1 else None,
        None,
        int(ts2[i]) if i < n2 else None,
        float(p2[i]) if i < n2 else None,
        None,
        int(ts3[i]) if i < n3 else None,
        float(p3[i]) if i < n3 else None,
        None,
        int(ts4[i]) if i < n4 else None,
        float(p4[i]) if i < n4 else None,
        None,
        int(ts5[i]) if i < n5 else None,
        float(p5[i]) if i < n5 else None,
        None,
        float(sum_submeters[i]) if i < n_sum else None,
        float(p1[i]) if i < n1 else None
    ]
    ws.append(row)
    if (i + 1) % 100000 == 0:
        print(f"  -> {i + 1:,} randuri scrise...")

wb.save(excel_path)
excel_size_mb = os.path.getsize(excel_path) / (1024 * 1024)
print(f"Fisier Excel salvat cu succes in {time.time() - t_excel:.2f} secunde! Marime: {excel_size_mb:.2f} MB")

# 4. Generare grafice cu pandas si matplotlib
print("\nGenerare grafice de consum...")
t_plots = time.time()

# Pregatim date cu datetime pentru axa X
for ch in range(1, 6):
    channels_data[ch]['datetime'] = pd.to_datetime(channels_data[ch]['timestamp'], unit='s')

colors = {
    1: '#2b5c8f',  # Albastru inchis / slate pentru Aggregate
    2: '#d9381e',  # Rosu aprins pentru Fierbator (Kettle)
    3: '#e67300',  # Portocaliu pentru Radiator (Heater)
    4: '#008080',  # Teal pentru Laptop
    5: '#7b2cbf',  # Mov pentru Videoproiector
    'total_sum': '#1b9e77', # Verde intens pentru Suma Aparatelor
    'aggregate': '#2b5c8f'
}

# --- Grafice individuale per consumator ---
for ch in range(1, 6):
    df_ch = channels_data[ch]
    name = labels.get(ch, f'Canal {ch}')
    friendly = friendly_names.get(ch, f'Canal {ch}')
    
    fig, ax = plt.subplots(figsize=(14, 5), dpi=150)
    ax.plot(df_ch['datetime'], df_ch['power'], color=colors[ch], linewidth=0.8, alpha=0.9, label=f'{name} (Canal {ch})')
    
    ax.set_title(f'Profil de Consum UK-DALE House 3: {friendly}', fontsize=13, fontweight='bold', pad=12)
    ax.set_xlabel('Data si Ora', fontsize=11, labelpad=8)
    ax.set_ylabel('Putere Activa [W]', fontsize=11, labelpad=8)
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%d %b %Y'))
    ax.grid(True, linestyle='--', alpha=0.5)
    
    # Statistici in caseta text
    stats_text = (
        f"Inregistrari: {len(df_ch):,}\n"
        f"Putere Medie: {df_ch['power'].mean():.2f} W\n"
        f"Putere Maxima: {df_ch['power'].max():.1f} W\n"
        f"Consum Estimat: {(df_ch['power'].sum() * 6 / 3600 / 1000):.2f} kWh"
    )
    ax.text(0.98, 0.92, stats_text, transform=ax.transAxes, verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white', edgecolor='#aaaaaa', alpha=0.9), fontsize=9)
    
    ax.legend(loc='upper left', frameon=True)
    plt.tight_layout()
    plot_file = os.path.join(OUT_PLOTS_DIR, f'grafic_canal_{ch}_{name}.png')
    plt.savefig(plot_file)
    plt.close()
    print(f"  -> Grafic salvat: {plot_file}")

# --- Grafic Consum Total Calculat (Suma consumatorilor monitorizati) ---
df_sum = pd.DataFrame({
    'datetime': channels_data[2]['datetime'].iloc[:common_rows],
    'power': sum_submeters
})

fig, ax = plt.subplots(figsize=(14, 5), dpi=150)
ax.plot(df_sum['datetime'], df_sum['power'], color=colors['total_sum'], linewidth=0.8, alpha=0.9, label='Suma Consumatorilor Monitorizati (Canale 2-5)')
ax.set_title('Consum Total Calculat UK-DALE House 3: Suma Aparatelor Monitorizate (Kettle + Heater + Laptop + Projector)', fontsize=13, fontweight='bold', pad=12)
ax.set_xlabel('Data si Ora', fontsize=11, labelpad=8)
ax.set_ylabel('Putere Activa [W]', fontsize=11, labelpad=8)
ax.xaxis.set_major_formatter(mdates.DateFormatter('%d %b %Y'))
ax.grid(True, linestyle='--', alpha=0.5)

stats_text = (
    f"Inregistrari: {len(df_sum):,}\n"
    f"Putere Medie: {df_sum['power'].mean():.2f} W\n"
    f"Putere Maxima: {df_sum['power'].max():.1f} W\n"
    f"Consum Total: {(df_sum['power'].sum() * 6 / 3600 / 1000):.2f} kWh"
)
ax.text(0.98, 0.92, stats_text, transform=ax.transAxes, verticalalignment='top', horizontalalignment='right',
        bbox=dict(boxstyle='round,pad=0.5', facecolor='white', edgecolor='#aaaaaa', alpha=0.9), fontsize=9)

ax.legend(loc='upper left', frameon=True)
plt.tight_layout()
plot_sum_file = os.path.join(OUT_PLOTS_DIR, 'grafic_consum_total_calculat_suma.png')
plt.savefig(plot_sum_file)
plt.close()
print(f"  -> Grafic salvat: {plot_sum_file}")

# --- Grafic Comparativ: Consum Total Masurat (Canal 1 Aggregate) vs Suma Aparatelor ---
fig, ax = plt.subplots(figsize=(14, 5), dpi=150)
ax.plot(channels_data[1]['datetime'], channels_data[1]['power'], color=colors[1], linewidth=0.7, alpha=0.7, label='Consum Total Masurat (Canal 1 - Aggregate / Mains)')
ax.plot(df_sum['datetime'], df_sum['power'], color=colors['total_sum'], linewidth=0.8, alpha=0.8, label='Suma Aparatelor Monitorizate (Canale 2-5)')

ax.set_title('Comparatie NILM: Consum Total Masurat (Mains) vs. Suma Aparatelor Monitorizate', fontsize=13, fontweight='bold', pad=12)
ax.set_xlabel('Data si Ora', fontsize=11, labelpad=8)
ax.set_ylabel('Putere Activa [W]', fontsize=11, labelpad=8)
ax.xaxis.set_major_formatter(mdates.DateFormatter('%d %b %Y'))
ax.grid(True, linestyle='--', alpha=0.5)
ax.legend(loc='upper right', frameon=True)

plt.tight_layout()
plot_comp_file = os.path.join(OUT_PLOTS_DIR, 'grafic_comparativ_total_masurat_vs_suma.png')
plt.savefig(plot_comp_file)
plt.close()
print(f"  -> Grafic salvat: {plot_comp_file}")

# --- Grafic Panou Multi-Plot cu Toti Consumatorii (Subplots) ---
fig, axes = plt.subplots(6, 1, figsize=(14, 16), sharex=True, dpi=150)

# Subplot 0: Aggregate
axes[0].plot(channels_data[1]['datetime'], channels_data[1]['power'], color=colors[1], linewidth=0.6)
axes[0].set_title('Canal 1: Consum Agregat Masurat (Aggregate Mains)', fontsize=10, fontweight='bold')
axes[0].set_ylabel('Putere [W]', fontsize=9)
axes[0].grid(True, linestyle='--', alpha=0.4)

# Subplot 1: Kettle
axes[1].plot(channels_data[2]['datetime'], channels_data[2]['power'], color=colors[2], linewidth=0.6)
axes[1].set_title('Canal 2: Fierbator de Apa (Kettle)', fontsize=10, fontweight='bold')
axes[1].set_ylabel('Putere [W]', fontsize=9)
axes[1].grid(True, linestyle='--', alpha=0.4)

# Subplot 2: Electric Heater
axes[2].plot(channels_data[3]['datetime'], channels_data[3]['power'], color=colors[3], linewidth=0.6)
axes[2].set_title('Canal 3: Calorifer Electric (Electric Heater)', fontsize=10, fontweight='bold')
axes[2].set_ylabel('Putere [W]', fontsize=9)
axes[2].grid(True, linestyle='--', alpha=0.4)

# Subplot 3: Laptop
axes[3].plot(channels_data[4]['datetime'], channels_data[4]['power'], color=colors[4], linewidth=0.6)
axes[3].set_title('Canal 4: Laptop', fontsize=10, fontweight='bold')
axes[3].set_ylabel('Putere [W]', fontsize=9)
axes[3].grid(True, linestyle='--', alpha=0.4)

# Subplot 4: Projector
axes[4].plot(channels_data[5]['datetime'], channels_data[5]['power'], color=colors[5], linewidth=0.6)
axes[4].set_title('Canal 5: Videoproiector (Projector)', fontsize=10, fontweight='bold')
axes[4].set_ylabel('Putere [W]', fontsize=9)
axes[4].grid(True, linestyle='--', alpha=0.4)

# Subplot 5: Consum Total Calculat
axes[5].plot(df_sum['datetime'], df_sum['power'], color=colors['total_sum'], linewidth=0.6)
axes[5].set_title('Consum Total Calculat (Suma Aparatelor Monitorizate: Canale 2-5)', fontsize=10, fontweight='bold')
axes[5].set_ylabel('Putere [W]', fontsize=9)
axes[5].set_xlabel('Data si Ora', fontsize=10, labelpad=6)
axes[5].grid(True, linestyle='--', alpha=0.4)
axes[5].xaxis.set_major_formatter(mdates.DateFormatter('%d %b %Y'))

plt.suptitle('Privire de Ansamblu UK-DALE House 3 - Toti Consumatorii si Consumul Total', fontsize=14, fontweight='bold', y=0.995)
plt.tight_layout()
plot_all_file = os.path.join(OUT_PLOTS_DIR, 'grafic_panou_toti_consumatorii.png')
plt.savefig(plot_all_file)
plt.close()
print(f"  -> Grafic salvat: {plot_all_file}")

# --- Grafic Zoom Detaliu 24 Ore (Semnatura aparatelor pe 2013-03-01) ---
start_zoom = pd.to_datetime('2013-03-01 00:00:00')
end_zoom = pd.to_datetime('2013-03-02 00:00:00')

fig, ax = plt.subplots(figsize=(14, 6), dpi=150)

# Masca pentru fiecare
mask1 = (channels_data[1]['datetime'] >= start_zoom) & (channels_data[1]['datetime'] <= end_zoom)
mask2 = (channels_data[2]['datetime'] >= start_zoom) & (channels_data[2]['datetime'] <= end_zoom)
mask3 = (channels_data[3]['datetime'] >= start_zoom) & (channels_data[3]['datetime'] <= end_zoom)
mask4 = (channels_data[4]['datetime'] >= start_zoom) & (channels_data[4]['datetime'] <= end_zoom)
mask5 = (channels_data[5]['datetime'] >= start_zoom) & (channels_data[5]['datetime'] <= end_zoom)

ax.plot(channels_data[1].loc[mask1, 'datetime'], channels_data[1].loc[mask1, 'power'], color='#888888', linewidth=1.0, linestyle='--', alpha=0.7, label='Aggregate Mains (Total Masurat)')
ax.plot(channels_data[2].loc[mask2, 'datetime'], channels_data[2].loc[mask2, 'power'], color=colors[2], linewidth=1.5, label='Kettle (Fierbator)')
ax.plot(channels_data[3].loc[mask3, 'datetime'], channels_data[3].loc[mask3, 'power'], color=colors[3], linewidth=1.2, label='Electric Heater (Calorifer)')
ax.plot(channels_data[4].loc[mask4, 'datetime'], channels_data[4].loc[mask4, 'power'], color=colors[4], linewidth=1.2, label='Laptop')
ax.plot(channels_data[5].loc[mask5, 'datetime'], channels_data[5].loc[mask5, 'power'], color=colors[5], linewidth=1.2, label='Projector (Videoproiector)')

ax.set_title('Detaliu Semnatura Energetica Aparate pe 24 Ore (01 Martie 2013) - UK-DALE House 3', fontsize=13, fontweight='bold', pad=12)
ax.set_xlabel('Ora (01 Martie 2013)', fontsize=11, labelpad=8)
ax.set_ylabel('Putere Activa [W]', fontsize=11, labelpad=8)
ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
ax.grid(True, linestyle='--', alpha=0.5)
ax.legend(loc='upper right', frameon=True)

plt.tight_layout()
plot_zoom_file = os.path.join(OUT_PLOTS_DIR, 'grafic_detaliu_semnatura_24ore.png')
plt.savefig(plot_zoom_file)
plt.close()
print(f"  -> Grafic salvat: {plot_zoom_file}")

print(f"\nGenerare grafice finalizata cu succes in {time.time() - t_plots:.2f} secunde!")
print("Toate operatiunile au fost incheiate cu succes.")
