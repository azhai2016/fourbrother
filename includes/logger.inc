<?php

class Logger
{
    public $file, $path, $level;

    private $stream;
    private $log_file_append = true;
    private $write_log = true;
    private $row_line_info = '';

    const DEBUG = 5;
    const CODE = 4;
    const INFO = 3;
    const WARN = 2;
    const ERROR = 1;
    const FATAL = 0;

    public function __construct($file, $level, $write_log = true, $log_file_append = true)
    {

        $out = floor((time() - filectime($file)) / 86400) > 0;
        if ($out) {
            $log_data = file_get_contents($file);
            $gzdata = gzencode($log_data, 9);
            file_put_contents($file . "-" . date('Ymd') . ".gz", $gzdata);
            unlink($file);
        }

        $this->file = $file;
        $this->write_log = $write_log;
        $this->log_file_append = $log_file_append;

        $this->level = $level;
        $this->path = $this->file;
        $this->start();

    }

    public function info($string, $name = '')
    {
        $this->row_line_info = '';
        return $this->log($string, $name, Logger::INFO);
    }

    public function warn($string, $name = '')
    {
        $this->row_line_info = '';
        return $this->log($string, $name, Logger::WARN);
    }

    public function code($string, $name = '')
    {
        $this->row_line_info = '';
        return $this->log($string, $name, Logger::CODE);
    }

    public function debug($string, $name = '')
    {
        $this->row_line_info = '';
        return $this->log($string, $name, Logger::DEBUG);
    }

    public function error($string, $name = null, $file = null, $line = null)
    {
        if (isset($file) && isset($line)) {
            $this->row_line_info = sprintf(' 文件名：%s 第 %d 行', $file, $line);
        } else {
            $this->row_line_info = '';
        }

        return $this->log($string, $name, Logger::ERROR);
    }

    public function fatal($string, $name = '')
    {
        $this->row_line_info = '';
        return $this->log($string, $name, Logger::FATAL);
    }

    public function clear()
    {
        $this->close();
        $this->open("w");
        $this->close();
        $this->open();
    }

    public function break_string($str, $num)
    {
        preg_match_all("/./u", $str, $arr); //将所有字符转成单个数组
        $strstr = '';
        $width = 0;
        $arr = $arr[0];
        foreach ($arr as $key => $string) {
            $strlen = strlen($string); //计算当前字符的长度，一个字母的长度为1，一个汉字的长度为3
            if ($strlen == 3) {
                $width += 1;
            } else {
                $width += 0.5;
            }
            $strstr .= $string;
            //计算当前字符的下一个
            if (array_key_exists($key + 1, $arr)) {
                $_strlen = strlen($arr[$key + 1]);
                if ($_strlen == 3) {
                    $_width = 1;
                } else {
                    $_width = 0.5;
                }
                if ($width + $_width > $num) {
                    $width = 0;
                    $strstr .= PHP_EOL;
                }
            }
        }
        return $strstr;
    }

    private function format_log_entry(array $log_entry): string
    {

        $log_line = "";

        if (!empty($log_entry)) {

            $log_entry = array_map(function ($v) {return print_r($v, true);}, $log_entry);

            $log_line .= date('H:m:s', $log_entry['timestamp']) . " ";

            $log_line .= "[" . strtoupper($log_entry['level']) . $this->row_line_info . "] : ";

            if (!empty($log_entry['name'])) {
                $log_line .= $log_entry['name'] . " => ";
            }

            if ($log_entry['level'] == 'CODE') {
                $log_line .= PHP_EOL . "\n" . PHP_EOL;
                $message = $log_entry['message'];
                // $message = SqlFormatter::compress($message);

                $log_line .= "\t" . $message;
                $log_line .= PHP_EOL . "\n" . PHP_EOL;

            } else {
                $log_line .= $log_entry['message'];
            }

        }

        return $log_line;
    }

    private function log($message, $name = null, $level = 4)
    {

        if ((int) $this->level < (int) $level) {
            return;
        }

        $level_name = '';

        switch ($level) {
            case 0:$level_name = 'FATAL';
                break;
            case 1:$level_name = 'ERROR';
                break;
            case 2:$level_name = 'WARN';
                break;
            case 3:$level_name = 'INFO';
                break;
            case 4:$level_name = 'CODE';
                break;
            case 5:$level_name = 'DEBUG';
                break;
            default:'';
        }

        $log_entry = [
            'timestamp' => time(),
            'name' => $name,
            'message' => $message,
            'level' => $level_name,
        ];

        $output_line = $this->format_log_entry($log_entry) . PHP_EOL;
        fputs($this->stream, $output_line);

    }

    private function start()
    {
        return $this->open();
    }

    private function open($mode = "a")
    {
        $mode = $this->log_file_append ? "a" : "w";
        if (true === $this->write_log) {
            $this->stream = fopen($this->path, $mode);
        }

    }

    private function close()
    {
        return fclose($this->stream);
    }

}
