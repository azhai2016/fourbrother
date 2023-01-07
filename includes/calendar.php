<?php

   
class Calendar 
{
    private $calendar;


    public function __construct(){
        $this->calendar = new \donatj\SimpleCalendar();
        $this->calendar->setStartOfWeek('Sunday');
        $this->calendar->setWeekDayNames([ '日', '一', '二', '三', '四', '五', '六' ]);
    }

    public function set_date($date){
        $this->calendar->setDate($date);
        return $this;
    }


    public function set_today($today=null){
        $this->calendar->setToday($today);
        return $this;
    }

    public function add_daily_html($html){

        $this->calendar->addDailyHtml($html[0],$html[1],$html[2]);
        return $this;
    }

   


    public function show(){
        return $this->calendar->show(true);
    }

}