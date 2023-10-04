<?php


use simitsdk\phpjasperxml\PHPJasperXML;

class Jasper_class
{

    public static function exoprt_pdf($mode, $jasper_file, $config, $paras = [])
    {
        $dir = $_SERVER['DOCUMENT_ROOT'] . '/company/' . user_company() . '/pdf_files';
        if (!file_exists($dir))
            mkdir($dir, 0777);

        $user_id = $_SESSION['wa_current_user']->user;

        $_file = $mode . '_' . $user_id . '.pdf'; //random_id() . '.pdf';
        $fname = $dir . '/' .  $_file;
        if (file_exists($fname)) {
            unlink($fname);
        }



        $report = new PHPJasperXML();
        $report->load_xml_file($jasper_file)
            ->setParameter($paras)
            ->setDataSource($config)
            ->export('Pdf', $fname);

        $file = company_path() . '/pdf_files/' . $_file;

        header('Location: ' . $file);
        exit;
    }
}
