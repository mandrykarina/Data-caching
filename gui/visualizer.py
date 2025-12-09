"""
LFU Cache Visualizer - Interactive GUI for LFU Cache Visualization
Works on Windows/Linux/Mac with Python 3.8+
"""

import tkinter as tk
from tkinter import ttk, messagebox
import json
from datetime import datetime
from cache_simulator import LFUCacheSimulator, AccessLog


class LFUCacheVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("LFU Cache Visualizer")
        self.root.geometry("1400x900")
        self.root.configure(bg="#f0f0f0")
        
        # Initialize cache simulator
        self.cache = LFUCacheSimulator(capacity=8)
        self.highlighted_key = None
        self.last_action = None
        
        # Configure styles
        self.setup_styles()
        
        # Build GUI
        self.build_ui()
        
        # Load initial data
        self.load_initial_data()
        
    def setup_styles(self):
        """Configure ttk styles"""
        style = ttk.Style()
        style.theme_use('clam')
        
        # Colors
        self.bg_color = "#f0f0f0"
        self.fg_color = "#333333"
        self.accent_color = "#0078d4"
        self.highlight_color = "#ffd700"
        self.error_color = "#d32f2f"
        self.success_color = "#388e3c"
        
        # Configure button style
        style.configure('TButton', font=('Segoe UI', 10))
        style.configure('Accent.TButton', font=('Segoe UI', 10, 'bold'))
        style.configure('TLabel', font=('Segoe UI', 10), background=self.bg_color)
        
    def build_ui(self):
        """Build main UI layout"""
        # Top control panel
        self.build_control_panel()
        
        # Main content area
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Left panel - Cache visualization
        self.build_cache_panel(main_frame)
        
        # Right panel - Info and logs
        self.build_info_panel(main_frame)
        
        # Bottom - Status bar
        self.build_status_bar()
        
    def build_control_panel(self):
        """Build top control panel with buttons"""
        control_frame = ttk.Frame(self.root)
        control_frame.pack(fill=tk.X, padx=10, pady=10)
        
        # Title
        title = ttk.Label(control_frame, text="🔍 LFU Cache Visualizer", 
                         font=('Segoe UI', 16, 'bold'))
        title.pack(side=tk.LEFT, padx=5)
        
        # Spacer
        ttk.Frame(control_frame).pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        # Control buttons
        btn_frame = ttk.Frame(control_frame)
        btn_frame.pack(side=tk.RIGHT)
        
        ttk.Button(btn_frame, text="⏭️  Step", command=self.step_action).pack(side=tk.LEFT, padx=3)
        ttk.Button(btn_frame, text="🔄 Reset", command=self.reset_action).pack(side=tk.LEFT, padx=3)
        ttk.Button(btn_frame, text="⚙️  Settings", command=self.open_settings).pack(side=tk.LEFT, padx=3)
        
    def build_cache_panel(self, parent):
        """Build left panel showing cache state"""
        left_frame = ttk.Frame(parent)
        left_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)
        
        # Cache visualization title
        ttk.Label(left_frame, text="Cache State", 
                 font=('Segoe UI', 12, 'bold')).pack(anchor=tk.W, pady=5)
        
        # Cache boxes frame (horizontal display of cache entries)
        self.cache_frame = ttk.Frame(left_frame)
        self.cache_frame.pack(fill=tk.BOTH, expand=True, pady=10)
        
        # Cache entry canvases
        self.cache_canvases = []
        self.cache_labels = []
        
        for i in range(self.cache.capacity):
            canvas = tk.Canvas(self.cache_frame, width=120, height=150, 
                              bg="white", relief=tk.SUNKEN, bd=1)
            canvas.pack(side=tk.LEFT, padx=5, pady=5, fill=tk.BOTH, expand=True)
            self.cache_canvases.append(canvas)
            
            label = ttk.Label(self.cache_frame, text="", font=('Segoe UI', 9))
            label.pack(side=tk.LEFT, padx=5)
            self.cache_labels.append(label)
        
        # Frequency table
        ttk.Label(left_frame, text="Frequency List", 
                 font=('Segoe UI', 12, 'bold')).pack(anchor=tk.W, pady=(20, 5))
        
        # Create treeview for frequency table
        columns = ('Key', 'Frequency', 'Last Access')
        self.freq_table = ttk.Treeview(left_frame, columns=columns, height=10, show='headings')
        
        for col in columns:
            self.freq_table.heading(col, text=col)
            self.freq_table.column(col, width=100, anchor=tk.CENTER)
        
        # Add scrollbar
        scrollbar = ttk.Scrollbar(left_frame, orient=tk.VERTICAL, command=self.freq_table.yview)
        self.freq_table.configure(yscroll=scrollbar.set)
        
        self.freq_table.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, pady=5)
        scrollbar.pack(side=tk.LEFT, fill=tk.Y)
        
    def build_info_panel(self, parent):
        """Build right panel with info and logs"""
        right_frame = ttk.Frame(parent)
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5)
        
        # Statistics section
        ttk.Label(right_frame, text="Statistics", 
                 font=('Segoe UI', 12, 'bold')).pack(anchor=tk.W, pady=5)
        
        self.stats_frame = ttk.Frame(right_frame)
        self.stats_frame.pack(fill=tk.X, pady=10)
        
        self.stats_labels = {}
        stats_names = ['Hits', 'Misses', 'Hit Rate', 'Evictions', 'Cache Size']
        
        for stat in stats_names:
            f = ttk.Frame(self.stats_frame)
            f.pack(fill=tk.X, pady=3)
            ttk.Label(f, text=f"{stat}:", width=15).pack(side=tk.LEFT)
            lbl = ttk.Label(f, text="0", font=('Segoe UI', 10, 'bold'), 
                           foreground=self.accent_color)
            lbl.pack(side=tk.LEFT, fill=tk.X, expand=True)
            self.stats_labels[stat] = lbl
        
        # Access log section
        ttk.Label(right_frame, text="Access Log", 
                 font=('Segoe UI', 12, 'bold')).pack(anchor=tk.W, pady=(20, 5))
        
        # Log text widget with scrollbar
        log_frame = ttk.Frame(right_frame)
        log_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        scrollbar = ttk.Scrollbar(log_frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.log_text = tk.Text(log_frame, height=15, width=40, 
                               yscrollcommand=scrollbar.set, font=('Courier', 9))
        self.log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.config(command=self.log_text.yview)
        
        # Configure text tags for colors
        self.log_text.tag_config('hit', foreground=self.success_color)
        self.log_text.tag_config('miss', foreground=self.error_color)
        self.log_text.tag_config('evict', foreground=self.accent_color)
        self.log_text.tag_config('step', foreground="#666666")
        
    def build_status_bar(self):
        """Build bottom status bar"""
        status_frame = ttk.Frame(self.root)
        status_frame.pack(fill=tk.X, side=tk.BOTTOM, padx=10, pady=5)
        
        self.status_label = ttk.Label(status_frame, text="Ready", relief=tk.SUNKEN)
        self.status_label.pack(fill=tk.X, side=tk.LEFT, expand=True)
        
    def load_initial_data(self):
        """Load initial cache data and request sequence"""
        # Predefined access sequence for demonstration
        self.access_sequence = [1, 2, 3, 4, 1, 2, 5, 1, 3, 6, 2, 7, 1, 4, 8, 1, 2, 3]
        self.current_step = 0
        
        self.update_display()
        
    def step_action(self):
        """Execute one step of cache access"""
        if self.current_step >= len(self.access_sequence):
            messagebox.showinfo("Done", "All steps completed!")
            return
        
        key = self.access_sequence[self.current_step]
        self.highlighted_key = key
        
        # Perform cache operation
        result = self.cache.get(key)
        
        # Add to log
        log_entry = f"[{self.current_step}] Access key={key}: {'HIT' if result['hit'] else 'MISS'}"
        self.add_log_entry(log_entry, 'hit' if result['hit'] else 'miss')
        
        if 'evicted' in result and result['evicted'] is not None:
            evict_log = f"       → Evicted key={result['evicted']} (freq={result['evicted_freq']})"
            self.add_log_entry(evict_log, 'evict')
        
        self.current_step += 1
        self.update_display()
        
    def reset_action(self):
        """Reset cache to initial state"""
        if messagebox.askyesno("Reset", "Reset cache to initial state?"):
            self.cache = LFUCacheSimulator(capacity=8)
            self.current_step = 0
            self.highlighted_key = None
            self.log_text.delete(1.0, tk.END)
            self.update_display()
            self.update_status("Cache reset")
        
    def open_settings(self):
        """Open settings dialog"""
        settings_window = tk.Toplevel(self.root)
        settings_window.title("Settings")
        settings_window.geometry("400x300")
        settings_window.resizable(False, False)
        
        ttk.Label(settings_window, text="Cache Capacity:", font=('Segoe UI', 10)).pack(pady=10)
        capacity_var = tk.IntVar(value=self.cache.capacity)
        ttk.Scale(settings_window, from_=4, to=16, orient=tk.HORIZONTAL,
                 variable=capacity_var).pack(fill=tk.X, padx=20, pady=5)
        
        ttk.Label(settings_window, text=f"Current: {self.cache.capacity}").pack()
        
        ttk.Label(settings_window, text="Access Sequence (comma-separated):").pack(pady=10)
        seq_text = tk.Text(settings_window, height=5, width=40)
        seq_text.insert(1.0, ",".join(map(str, self.access_sequence)))
        seq_text.pack(padx=20, pady=5)
        
        def apply_settings():
            try:
                new_capacity = capacity_var.get()
                new_seq = list(map(int, seq_text.get(1.0, tk.END).strip().split(',')))
                
                self.cache = LFUCacheSimulator(capacity=new_capacity)
                self.access_sequence = new_seq
                self.current_step = 0
                self.log_text.delete(1.0, tk.END)
                self.update_display()
                settings_window.destroy()
                messagebox.showinfo("Success", "Settings applied!")
            except ValueError:
                messagebox.showerror("Error", "Invalid input format")
        
        ttk.Button(settings_window, text="Apply", command=apply_settings).pack(pady=10)
        
    def update_display(self):
        """Update all visual elements"""
        self.update_cache_visualization()
        self.update_frequency_table()
        self.update_statistics()
        self.update_status(f"Step {self.current_step}/{len(self.access_sequence)}")
        
    def update_cache_visualization(self):
        """Draw cache boxes with current state"""
        cache_state = self.cache.get_cache_state()
        
        for i in range(self.cache.capacity):
            canvas = self.cache_canvases[i]
            canvas.delete("all")
            
            if i < len(cache_state):
                entry = cache_state[i]
                key = entry['key']
                freq = entry['frequency']
                
                # Determine color
                if self.highlighted_key == key:
                    bg_color = self.highlight_color
                    text_color = "#000000"
                else:
                    bg_color = "#e3f2fd"
                    text_color = self.fg_color
                
                # Draw box
                canvas.create_rectangle(5, 5, 115, 145, fill=bg_color, outline=self.accent_color, width=2)
                
                # Draw text
                canvas.create_text(60, 30, text=f"Key: {key}", font=('Segoe UI', 11, 'bold'), fill=text_color)
                canvas.create_text(60, 60, text=f"Freq: {freq}", font=('Segoe UI', 10), fill=text_color)
                canvas.create_text(60, 85, text=f"Index: {i}", font=('Segoe UI', 9), fill=text_color)
                
                # Draw frequency bar
                bar_width = (freq / 10) * 100  # Normalize to cache size
                canvas.create_rectangle(10, 115, 10 + bar_width, 135, fill=self.accent_color)
                canvas.create_rectangle(10, 115, 110, 135, outline=self.fg_color)
            else:
                # Empty slot
                canvas.create_rectangle(5, 5, 115, 145, fill="white", outline="#cccccc", width=1, dash=(2, 2))
                canvas.create_text(60, 75, text="Empty", font=('Segoe UI', 10, 'italic'), fill="#999999")
        
        self.highlighted_key = None
        
    def update_frequency_table(self):
        """Update frequency table"""
        # Clear existing items
        for item in self.freq_table.get_children():
            self.freq_table.delete(item)
        
        # Add entries from cache
        cache_state = self.cache.get_cache_state()
        for entry in cache_state:
            last_access = entry['last_access'].strftime("%H:%M:%S") if entry['last_access'] else "N/A"
            self.freq_table.insert('', tk.END, values=(
                entry['key'],
                entry['frequency'],
                last_access
            ))
        
    def update_statistics(self):
        """Update statistics panel"""
        stats = self.cache.get_statistics()
        
        self.stats_labels['Hits'].config(text=str(stats['hits']))
        self.stats_labels['Misses'].config(text=str(stats['misses']))
        self.stats_labels['Hit Rate'].config(text=f"{stats['hit_rate']:.1f}%")
        self.stats_labels['Evictions'].config(text=str(stats['evictions']))
        self.stats_labels['Cache Size'].config(text=f"{stats['cache_size']}/{self.cache.capacity}")
        
    def add_log_entry(self, text, tag='step'):
        """Add entry to log"""
        self.log_text.insert(tk.END, text + '\n', tag)
        self.log_text.see(tk.END)  # Auto-scroll to bottom
        
    def update_status(self, text):
        """Update status bar"""
        self.status_label.config(text=text)
        self.root.update_idletasks()


def main():
    root = tk.Tk()
    app = LFUCacheVisualizer(root)
    root.mainloop()


if __name__ == "__main__":
    main()