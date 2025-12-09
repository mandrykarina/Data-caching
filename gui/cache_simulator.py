"""
LFU Cache Simulator - Pure Python implementation of LFU Cache
For use with visualizer.py
"""

from datetime import datetime
from collections import defaultdict
from typing import Dict, List, Optional, Any


class AccessLog:
    """Log entry for cache access"""
    def __init__(self, key: int, hit: bool, timestamp: datetime = None):
        self.key = key
        self.hit = hit
        self.timestamp = timestamp or datetime.now()
        self.evicted_key = None
        self.evicted_freq = None


class CacheEntry:
    """Single cache entry"""
    def __init__(self, key: int, value: Any = None):
        self.key = key
        self.value = value
        self.frequency = 0
        self.last_access = None


class LFUCacheSimulator:
    """
    LFU (Least Frequently Used) Cache implementation
    Mirrors the C++ CacheManager behavior for visualization
    """
    
    def __init__(self, capacity: int = 8):
        self.capacity = capacity
        self.cache_map: Dict[int, CacheEntry] = {}  # key -> CacheEntry
        self.freq_lists: Dict[int, List[int]] = defaultdict(list)  # freq -> [keys] (front: most recent)
        self.min_freq = 0
        self.access_log: List[AccessLog] = []
        
        # Statistics
        self.hits = 0
        self.misses = 0
        self.evictions = 0
        self.total_accesses = 0
        
    def get(self, key: int) -> Dict[str, Any]:
        """
        Get value from cache
        Returns dict with: {'hit': bool, 'evicted': int or None, 'evicted_freq': int or None}
        """
        self.total_accesses += 1
        evicted = None
        evicted_freq = None
        
        if key in self.cache_map:
            # Cache hit
            self.hits += 1
            entry = self.cache_map[key]
            self._touch(key)
            
            log = AccessLog(key, True)
            self.access_log.append(log)
            return {'hit': True, 'evicted': None, 'evicted_freq': None}
        else:
            # Cache miss
            self.misses += 1
            
            # Need to add to cache - evict if necessary
            if len(self.cache_map) >= self.capacity:
                evicted, evicted_freq = self._evict_one()
                evicted_key = evicted
            
            # Add to cache
            entry = CacheEntry(key, f"value_{key}")
            entry.frequency = 1
            entry.last_access = datetime.now()
            self.cache_map[key] = entry
            self.freq_lists[1].insert(0, key)  # Insert at front (most recent)
            self.min_freq = 1
            
            log = AccessLog(key, False)
            if 'evicted_key' in locals():
                log.evicted_key = evicted_key
                log.evicted_freq = evicted_freq
            self.access_log.append(log)
            
            return {
                'hit': False,
                'evicted': evicted,
                'evicted_freq': evicted_freq
            }
    
    def _touch(self, key: int):
        """Increase frequency of a key"""
        if key not in self.cache_map:
            return
        
        entry = self.cache_map[key]
        old_freq = entry.frequency
        
        # Remove from old frequency list
        if old_freq in self.freq_lists:
            try:
                self.freq_lists[old_freq].remove(key)
            except ValueError:
                pass
            
            # If old list is empty and it was min_freq, increment min_freq
            if len(self.freq_lists[old_freq]) == 0 and old_freq == self.min_freq:
                self.min_freq += 1
        
        # Add to new frequency list at front (most recent)
        new_freq = old_freq + 1
        entry.frequency = new_freq
        entry.last_access = datetime.now()
        self.freq_lists[new_freq].insert(0, key)
    
    def _evict_one(self) -> tuple:
        """
        Evict least frequently used item
        If multiple items have same frequency, evict least recently used among them
        Returns (evicted_key, evicted_freq)
        """
        if not self.cache_map:
            return None, None
        
        # Find list for min_freq
        if self.min_freq not in self.freq_lists or not self.freq_lists[self.min_freq]:
            # Find next non-empty frequency
            for freq in sorted(self.freq_lists.keys()):
                if self.freq_lists[freq]:
                    self.min_freq = freq
                    break
        
        # Evict least recently used from min_freq list (back of list)
        freq_list = self.freq_lists.get(self.min_freq, [])
        if not freq_list:
            # Fallback: evict any key
            victim_key = next(iter(self.cache_map.keys()))
            victim_freq = self.cache_map[victim_key].frequency
        else:
            victim_key = freq_list[-1]  # Least recently used (back)
            victim_freq = self.min_freq
            freq_list.pop()  # Remove from list
        
        # Remove from cache
        if victim_key in self.cache_map:
            del self.cache_map[victim_key]
        
        self.evictions += 1
        return victim_key, victim_freq
    
    def get_cache_state(self) -> List[Dict[str, Any]]:
        """Get current cache state for visualization"""
        result = []
        
        # Sort by frequency (descending) then by recency
        sorted_entries = sorted(
            self.cache_map.items(),
            key=lambda x: (-x[1].frequency, -x[1].last_access.timestamp())
        )
        
        for key, entry in sorted_entries:
            result.append({
                'key': key,
                'value': entry.value,
                'frequency': entry.frequency,
                'last_access': entry.last_access
            })
        
        return result
    
    def get_frequency_list(self) -> Dict[int, List[int]]:
        """Get frequency lists (for debugging/visualization)"""
        return dict(self.freq_lists)
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get cache statistics"""
        hit_rate = (self.hits / self.total_accesses * 100) if self.total_accesses > 0 else 0
        
        return {
            'hits': self.hits,
            'misses': self.misses,
            'total_accesses': self.total_accesses,
            'hit_rate': hit_rate,
            'evictions': self.evictions,
            'cache_size': len(self.cache_map),
            'capacity': self.capacity
        }
    
    def get_access_log(self) -> List[AccessLog]:
        """Get full access log"""
        return self.access_log.copy()
    
    def reset(self):
        """Reset cache to initial state"""
        self.cache_map.clear()
        self.freq_lists.clear()
        self.access_log.clear()
        self.min_freq = 0
        self.hits = 0
        self.misses = 0
        self.evictions = 0
        self.total_accesses = 0
    
    def clear(self):
        """Alias for reset"""
        self.reset()
    
    def __repr__(self):
        return f"LFUCache(size={len(self.cache_map)}/{self.capacity}, hits={self.hits}, misses={self.misses})"