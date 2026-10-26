function [d] = JahresTag(Monat,Tag)
MTage = [ 31 28 31 30 31 30 31 31 30 31 30 31];
d = Tag;
if(Monat > 1)
    for i=1:Monat-1
        d = d+MTage(i);
    end
end
end
