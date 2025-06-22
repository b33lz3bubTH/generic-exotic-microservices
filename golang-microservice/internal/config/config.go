package config

import (
	"os"
	"sync"

	"github.com/spf13/viper"
)

type RateLimitSetting struct {
	Post int `mapstructure:"post"`
	Get  int `mapstructure:"get"`
}

type RateLimitConfig struct {
	Default     RateLimitSetting            `mapstructure:"default"`
	Controllers map[string]RateLimitSetting `mapstructure:"controllers"`
}

type Config struct {
	DBFile    string
	RateLimit RateLimitConfig
}

var (
	cfg  *Config
	once sync.Once
)

func loadConfig() *Config {
	v := viper.New()
	v.SetConfigName("config")
	v.SetConfigType("yaml")
	v.AddConfigPath(".")
	_ = v.BindEnv("DB_FILE")
	v.SetDefault("dbfile", os.Getenv("DB_FILE"))
	v.SetDefault("ratelimit.default.post", 300)
	v.SetDefault("ratelimit.default.get", 100)
	if err := v.ReadInConfig(); err != nil {
		// fallback to env/defaults
	}
	var c Config
	if err := v.Unmarshal(&c); err != nil {
		panic(err)
	}
	if c.DBFile == "" {
		c.DBFile = v.GetString("dbfile")
	}
	return &c
}

func Get() *Config {
	once.Do(func() {
		cfg = loadConfig()
	})
	return cfg
}
