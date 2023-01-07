var selectChange = {
    '#Add_content': function (e) {
        e.onclick = function () {
            var content = $('textarea[name="FormulaContent"]').val();
            var targent = $('#TargetId').val();
            $('textarea[name="FormulaContent"]').val(content + '[' + targent + ']');
        };
    },

    '#Add_select_content': function (e) {
        var p = $('.condition_master').find('button');
        var n = p.parent().find('select');

        p.each(function (a) {

            $(this).on('click', function () {
                var n = $(this).parent().find('select option:selected');

                var s = '<tr><td></td><td>' + n.val() + '_' + n.text().trim() + '</td></tr>';
                $('.condition_list').append(s);
            });
        });

    }
}


Behaviour.register(selectChange);